/*
*********************************************************************************************************
*                                              led_ctrl.h
*
*  Module: Control LED by push button on ZedBoard (BTN0 = counter increment, BTN1 = reset)
*  Description: Create a LED task running in parallel in the uC/OS-III system.
*
*********************************************************************************************************
*/

#ifndef  LED_CTRL_H
#define  LED_CTRL_H

#include  <stdint.h>    /* Dùng cho uint32_t */

/*
*********************************************************************************************************
*                                          CÁC HÀM GIAO TIẾP
*********************************************************************************************************
*/

/**
 * @brief   Tạo task điều khiển LED (BTN0 đếm, BTN1 reset).
 *
 * @param   prio  Độ ưu tiên của task (số càng nhỏ => ưu tiên càng cao).
 *
 * @note    Gọi hàm này trong MainTask (sau khi UCOSStartup() đã khởi chạy kernel).
 */
void LedCtrlTask_Create(uint32_t prio);

#endif  /* LED_CTRL_H */

/*
*********************************************************************************************************
*                                               END
*********************************************************************************************************
*/
