// led_task_osal.c
#include "hw_config.h"
#include "osal.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xgpio.h"

// fwd decl (từ 2 file OSAL ở trên)
int  BtnIrq_Init_OSAL(XGpio *gpio, OSAL_Sem *sem);

// static XGpio    *s_gpio;
static OSAL_Sem *s_sem;
static uint32_t  s_led_sw;
XGpio g_gpio;

static void LedTask(void *arg){
    (void)arg;
    // init LED = 0
    XGpio_DiscreteWrite(&g_gpio, LED_CH, 0x00);
    s_led_sw = 0;

    if (OSAL_Log()) OSAL_Log()("LED task (OSAL) started. Waiting BTN...\r\n");

    for(;;){
        OSAL_SemPend(s_sem, 0);
        // Toggle bit0
        s_led_sw ^= 0x01;
        uint32_t cur = XGpio_DiscreteRead(&g_gpio, LED_CH);
        cur = (cur & ~0x01u) | (s_led_sw & 0x01u);
        XGpio_DiscreteWrite(&g_gpio, LED_CH, cur);

        if (OSAL_Log()) OSAL_Log()("[LED] toggled -> 0x%02lx\r\n", (unsigned long)cur);

        // Debounce ~50ms rồi mở lại IRQ
        OSAL_SleepMs(50);
        extern void BtnIrq_ReEnable_OSAL(XGpio *gpio);
        BtnIrq_ReEnable_OSAL(&g_gpio);
    }
}

void Demo_irpt(void){

    // 2) Init GPIO chung cho LED & BTN
    int st = XGpio_Initialize(&g_gpio, BTN_GPIO_DEV_ID);
    xil_printf("XGpio_Initialize = %d\r\n", st);
    XGpio_SetDataDirection(&g_gpio, LED_CH, 0x00);
    XGpio_SetDataDirection(&g_gpio, BTN_CH, 0xFF);

    // Clear pending/disable trước khi enable (LEVEL_HIGH)
    XGpio_InterruptDisable(&g_gpio, XGPIO_IR_CH1_MASK | XGPIO_IR_CH2_MASK);
    XGpio_InterruptGlobalDisable(&g_gpio);
    XGpio_InterruptClear(&g_gpio,   XGPIO_IR_CH1_MASK | XGPIO_IR_CH2_MASK);

    // 3) Sem = 0
    OSAL_SemCreate(&s_sem, 0);

    // 4) Kết nối BTN IRQ → ISR (OSAL) và bật IRQ
    int ok_irq = BtnIrq_Init_OSAL(&g_gpio, s_sem);
    xil_printf("BtnIrq_Init_OSAL = %d\r\n", ok_irq);

    // 5) Tạo LED task (OSAL)
    int ok_task = OSAL_TaskCreate(NULL, "led_task_osal", LedTask, 0, 1024, 10);
    xil_printf("LedTask_Create_OSAL = %d\r\n", ok_task);

    xil_printf("Press BTN0 (CH%d, bit0) to toggle LED on CH%d.\r\n", BTN_CH, LED_CH);

}