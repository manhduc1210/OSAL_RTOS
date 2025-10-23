#ifndef PRODUCER_H
#define PRODUCER_H

#include <stdint.h>
#include <Source/os.h>

// ===== Cấu hình Task
#define PROD_TASK2_PRIO        5u   // Consumer (cao nhất để thấy preempt)
#define PROD_TASK1_PRIO        6u   // Producer A
#define PROD_TASK3_PRIO        7u   // Producer B

#define PROD_STK_SIZE       1024u
#define PROD_Q_DEPTH          16u

// ===== Tốc độ chu kỳ (ms)
#define PROD_T1_PERIOD_MS   300u   // Task1: 300 ms/lần
#define PROD_T3_PERIOD_MS   500u   // Task3: 500 ms/lần
#define PROD_T2_WORK_MS      50u   // Task2: 50 ms/lần (giả lập xử lý)

// ===== API chính
void Producer_Init(void);

// ===== Hook phần cứng BTN0 (tùy platform)
typedef void (*btn_isr_t)(void *ref);
void btn0_hw_init(btn_isr_t isr_cb, void *ref);
void clear_btn0_irq(void);

#endif /* PRODUCER_H */

