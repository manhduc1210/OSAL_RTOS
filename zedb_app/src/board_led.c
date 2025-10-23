#include "board_led.h"
#include "osal.h"
#include "xgpio.h"
#include "xparameters.h"

#define LED_GPIO_DEVICE_ID   XPAR_AXI_GPIO_0_DEVICE_ID
#define LED_CHANNEL          1
#define LED_MASK             0xFF   // 8 LED

static XGpio LedGpio;

void BoardLed_Init(void)
{
    int status = XGpio_Initialize(&LedGpio, LED_GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        OSAL_LOG("[LED] XGpio init failed\r\n");
        return;
    }
    XGpio_SetDataDirection(&LedGpio, LED_CHANNEL, 0x00); // tất cả output
    XGpio_DiscreteWrite(&LedGpio, LED_CHANNEL, 0x00);
    OSAL_LOG("[LED] Initialized on GPIO #%d\r\n", LED_GPIO_DEVICE_ID);
}

void BoardLed_Set(uint8_t on)
{
    static uint8_t state = 0;
    if (on)
        state = 0xFF;
    else
        state = 0x00;

    XGpio_DiscreteWrite(&LedGpio, LED_CHANNEL, state);
}
