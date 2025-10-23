#ifndef ISR_BTN_H
#define ISR_BTN_H

#include "xgpio.h"
#include <Source/os.h>
#include  "xscugic.h"

// Khởi tạo IRQ cho BTN: kết nối ISR vào GIC, bật IRQ trong GPIO
CPU_BOOLEAN BtnIrq_Init(XGpio *pGpio, XScuGic *pGic, OS_SEM *pSem);

// gọi từ task sau debounce để re-enable IRQ
void BtnIrq_ReEnable(XGpio *pGpio);

#endif // ISR_BTN_H
