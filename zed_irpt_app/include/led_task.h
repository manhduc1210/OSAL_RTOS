#ifndef LED_TASK_H
#define LED_TASK_H

#include "xgpio.h"
#include <Source/os.h>

// Tạo task điều khiển LED: chờ semaphore từ ISR rồi toggle LED
CPU_BOOLEAN LedTask_Create(XGpio *pGpio, OS_SEM *pSem);

#endif // LED_TASK_H
