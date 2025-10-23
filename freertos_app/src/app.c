#include <xil_printf.h>
#include "osal.h"

// demo
void Demo1_Start(void);

int main(void)
{
    OSAL_Config cfg = {
        .backend      = OSAL_BACKEND_FREERTOS,
        .log          = xil_printf,
        .platform_ctx = NULL
    };
    if (OSAL_Init(&cfg) != OSAL_OK) {
        xil_printf("OSAL_Init failed\r\n");
        for(;;);
    }

    xil_printf("\r\n=== Demo 1: Blink + Log via OSAL (FreeRTOS) ===\r\n");
    Demo1_Start();                //(Blink/Log)

    vTaskStartScheduler();        // REQUIRED: start the FreeRTOS scheduler

    // If you get here, it means there is not enough heap for Idle/Timer task.
    xil_printf("Scheduler failed to start (heap?)\r\n");
    for(;;);
}
