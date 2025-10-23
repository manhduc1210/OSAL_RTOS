#include <Source/os.h>
#include <ucos_bsp.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"
#include  <led_ctrl.h>

/*================ Config Task =================*/
#define LED_TASK_STK_SIZE  1024u  /* Stack: 1024 words = ~4 KB */
#define BTN_DEBOUNCE_MS    20u    /* Button vibration filter ~20ms */

static OS_TCB   LedTaskTCB;
static CPU_STK  LedTaskStk[LED_TASK_STK_SIZE];
static void LedTask(void *p_arg);

/*================================================*/
void LedCtrlTask_Create(uint32_t prio)
{
    OS_ERR err;

    OSTaskCreate(&LedTaskTCB,
                 "LED Counter Task",
                 LedTask,
                 0,
                 prio,
                 &LedTaskStk[0],
                 LED_TASK_STK_SIZE / 10u,
                 LED_TASK_STK_SIZE,
                 0u, 0u, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);

    if (err != OS_ERR_NONE)
        UCOS_Print("LedCtrl: create failed\r\n");
    else
        UCOS_Print("LedCtrl: created\r\n");
}

/*================================================*/
static void LedTask(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;

    XGpio gpio;
    u32   counter = 0;
    u32   prev_btn0 = 0, prev_btn1 = 0;
    u32   btn_val = 0;

    /* 1. Init AXI GPIO */
    if (XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID) != XST_SUCCESS) {
        UCOS_Print("LedCtrl: XGpio init failed\r\n");
        for (;;); /* dừng tại chỗ nếu lỗi */
    }

    /* 2. Channel direction */
    XGpio_SetDataDirection(&gpio, 1, 0x00u);  /* LED out */
    XGpio_SetDataDirection(&gpio, 2, 0xFFu);  /* BTN in  */

    UCOS_Print("LedCtrl: running <BTN0=count, BTN1=reset>\r\n");

    while (DEF_TRUE) {
        /* Read 5 buttons (only use BTN0 and BTN1) */
        btn_val = XGpio_DiscreteRead(&gpio, 2) & 0x1Fu;

        u32 btn0 = (btn_val >> 0) & 0x1;
        u32 btn1 = (btn_val >> 1) & 0x1;

        /* BTN active-high hay active-low?
           Trên ZedBoard là active-high: nhấn = 1 */
        if (btn0 && !prev_btn0) {      /* rising edge BTN0 */
            counter++;
            if (counter > 255) counter = 0;
            xil_printf("Counter: %u\r\n", counter);
        }

         /* BTN1: reset counter */
        if (btn1 && !prev_btn1) {      /* rising edge BTN1 */
            counter = 0;
            UCOS_Print("Counter reset\r\n");
        }

        prev_btn0 = btn0;
        prev_btn1 = btn1;

        /* Display counter on LED (8 bit) */
        XGpio_DiscreteWrite(&gpio, 1, counter & 0xFFu);

        /* Delay ngắn để debounce & polling */
        OSTimeDlyHMSM(0, 0, 0, BTN_DEBOUNCE_MS, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}
