// app_osal.c
#include <ucos_bsp.h>       // để gọi UCOSStartup()
#include "xgpio.h"
#include "xil_printf.h"

#include "hw_config.h"
#include "gic_util.h"
#include "osal.h"

extern XGpio g_gpio;
// static OSAL_Sem *g_btn_sem;

void Demo_irpt(void);

void MainTask(void *p_arg){
    (void)p_arg;

    xil_printf("\r\n=== Demo (OSAL): BTN IRQ -> Semaphore -> LED Task ===\r\n");

    // Init OSAL với backend uC/OS-III + log + GIC platform_ctx
    XScuGic *p_gic = GIC_GetInstance(); // từ gic_util.c
    OSAL_Config cfg = {
        .backend      = OSAL_BACKEND_UCOS3,
        .log          = xil_printf,
        .platform_ctx = (void*)p_gic,
    };
    OSAL_Init(&cfg);
    Demo_irpt();

    // Loop for monitor
    for(;;){
        uint32_t leds = XGpio_DiscreteRead(&g_gpio, LED_CH);
        uint32_t btn  = XGpio_DiscreteRead(&g_gpio, BTN_CH);
        xil_printf("Monitor: LED=0x%02lx BTN=0x%02lx\r\n",
                   (unsigned long)leds, (unsigned long)btn);
        OSAL_SleepMs(1000);
    }
}

int main(void){
    UCOSStartup(MainTask);
    return 0;
}
