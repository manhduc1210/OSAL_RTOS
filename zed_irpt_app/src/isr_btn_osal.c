// isr_btn_osal.c
#include "hw_config.h"
#include "osal.h"
#include "xgpio.h"

typedef struct {
    XGpio    *gpio;
    OSAL_Sem *sem;
} BtnIsrCtx;

static BtnIsrCtx s_ctx;
static volatile int s_masked = 0;

static void BtnIsr(void *ref){
    (void)ref;
    OSAL_EnterIsr();

    // 1) Chặn lặp tức thì & 2) clear pending (LEVEL_HIGH)
    XGpio_InterruptDisable(s_ctx.gpio, BTN_IRQ_MASK);
    XGpio_InterruptClear  (s_ctx.gpio, BTN_IRQ_MASK);

    // 3) Đánh thức task
    OSAL_SemPost(s_ctx.sem);
    s_masked = 1;

    OSAL_ExitIsr();
}

int BtnIrq_Init_OSAL(XGpio *gpio, OSAL_Sem *sem){
    s_ctx.gpio = gpio; s_ctx.sem = sem;

    // Kết nối vào GIC qua OSAL (prio ~0xA0, LEVEL_HIGH=3)
    if (OSAL_IrqConnect(BTN_GPIO_INT_ID, BtnIsr, gpio, 0xA0, 3) != 0)
        return -1;
    OSAL_IrqEnable(BTN_GPIO_INT_ID);

    // Bật interrupt trong GPIO: Global + kênh BTN
    XGpio_InterruptGlobalEnable(gpio);
    XGpio_InterruptEnable     (gpio, BTN_IRQ_MASK);
    return 0;
}

void BtnIrq_ReEnable_OSAL(XGpio *gpio){
    (void)s_masked; s_masked = 0;
    XGpio_InterruptEnable(gpio, BTN_IRQ_MASK);
}
