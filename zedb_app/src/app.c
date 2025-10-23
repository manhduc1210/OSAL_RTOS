#include "osal_task.h"
#include <ucos_bsp.h>
#include "osal.h"
#include <xil_printf.h>
// forward
void Demo1_Start(void);
void MainTask(void *p_arg);

int main(void) {
    UCOSStartup(MainTask);
    return 0;
}

void MainTask(void *p_arg) {
    (void)p_arg;

    OSAL_Config cfg = {
        .backend = OSAL_BACKEND_UCOS3,
        .log = xil_printf,         // dùng UART của Zynq
        .platform_ctx = NULL
    };
    if (OSAL_Init(&cfg) != OSAL_OK) {
        xil_printf("OSAL_Init failed\r\n");
        return;
    }

    xil_printf("\r\n=== Demo 1: Blink + Log via OSAL ===\r\n");
    Demo1_Start();

}

