#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "xparameters.h"
#include "xgpio.h"
#include "xgpio_l.h"

#define BTN_GPIO_DEV_ID   XPAR_AXI_GPIO_0_DEVICE_ID
#define BTN_GPIO_INT_ID   XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

// CH mapping trong cùng 1 AXI GPIO
#define BTN_CH            2   // input
#define LED_CH            1   // output
#define BTN_MASK_BITS     0x01

#if   (BTN_CH == 1)
  #define BTN_IRQ_MASK    XGPIO_IR_CH1_MASK
#elif (BTN_CH == 2)
  #define BTN_IRQ_MASK    XGPIO_IR_CH2_MASK
#else
  #error "BTN_CH must be 1 or 2"
#endif

#ifndef XGPIO_GIE_OFFSET
#define XGPIO_GIE_OFFSET  0x11C
#endif
#ifndef XGPIO_ISR_OFFSET
#define XGPIO_ISR_OFFSET  0x120
#endif
#ifndef XGPIO_IER_OFFSET
#define XGPIO_IER_OFFSET  0x128
#endif
#ifndef XGPIO_IR_CH1_MASK
#define XGPIO_IR_CH1_MASK 0x00000001U
#endif
#ifndef XGPIO_IR_CH2_MASK
#define XGPIO_IR_CH2_MASK 0x00000002U
#endif

#endif // HW_CONFIG_H
