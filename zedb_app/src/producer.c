#include "producer.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include <stdlib.h>
// =============================
// IPC & trạng thái
// =============================
static OS_Q    g_q_data;
static OS_Q    g_q_alt;
static OS_SEM  g_sem_data;
static OS_SEM  g_sem_alt;
static volatile CPU_INT08U g_mode = 0;   // 0: Task1, 1: Task3

// =============================
// TCB & Stack
// =============================
static OS_TCB  g_tcb_task1, g_tcb_task2, g_tcb_task3;
static CPU_STK g_stk_task1[PROD_STK_SIZE];
static CPU_STK g_stk_task2[PROD_STK_SIZE];
static CPU_STK g_stk_task3[PROD_STK_SIZE];

// =============================
// Forward
// =============================
static void Task1_Producer(void *p_arg);
static void Task2_Consumer(void *p_arg);
static void Task3_Producer(void *p_arg);
static inline uint32_t next_wave(void);
static void BtnTask_Poll(void *p_arg);

// =============================
// GPIO BTN
// =============================
static XGpio s_GpioBtn;
#define BTN_GPIO_DEV_ID  XPAR_AXI_GPIO_0_DEVICE_ID
#define BTN_CH           2
#define BTN0_MASK        0x1u   // bit 0

static inline uint8_t btn0_read(void)
{
    u32 v = XGpio_DiscreteRead(&s_GpioBtn, BTN_CH);
    return (v & BTN0_MASK) ? 1u : 0u;
}

// =============================
// Flush backlog khi đổi mode
// =============================
static void flush_src(OS_Q *pq, OS_SEM *psem)
{
    OS_ERR err;
    OS_MSG_SIZE msg_size;

    // Xả semaphore về 0
    for (;;) {
        OSSemPend(psem, 0, OS_OPT_PEND_NON_BLOCKING, 0, &err);
        if (err != OS_ERR_NONE) break;
    }

    // Xả queue: lấy hết các message đang chờ (non-blocking)
    for (;;) {
        void *p = OSQPend(pq, 0, OS_OPT_PEND_NON_BLOCKING, &msg_size, 0, &err);
        if (err != OS_ERR_NONE) break;
        (void)p;
    }
}

// =============================
// Init hệ thống
// =============================
void Producer_Init(void)
{
    OS_ERR err;
    int status;

    status = XGpio_Initialize(&s_GpioBtn, BTN_GPIO_DEV_ID);
    if (status != XST_SUCCESS)
        xil_printf("[HW] XGpio Init BTN failed=%d\r\n", status);

    XGpio_SetDataDirection(&s_GpioBtn, BTN_CH, 0xFFFFFFFFu); // input

    // IPC
    OSQCreate(&g_q_data, "q_data", PROD_Q_DEPTH, &err);
    OSQCreate(&g_q_alt,  "q_alt",  PROD_Q_DEPTH, &err);
    OSSemCreate(&g_sem_data, "sem_data", 0, &err);
    OSSemCreate(&g_sem_alt,  "sem_alt",  0, &err);

    // Task
    OSTaskCreate(&g_tcb_task2,"Task2_Consumer",Task2_Consumer,0,PROD_TASK2_PRIO,
                 g_stk_task2,PROD_STK_SIZE/10,PROD_STK_SIZE,0,0,0,
                 OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,&err);

    OSTaskCreate(&g_tcb_task1,"Task1_Producer",Task1_Producer,0,PROD_TASK1_PRIO,
                 g_stk_task1,PROD_STK_SIZE/10,PROD_STK_SIZE,0,0,0,
                 OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,&err);

    OSTaskCreate(&g_tcb_task3,"Task3_Producer",Task3_Producer,0,PROD_TASK3_PRIO,
                 g_stk_task3,PROD_STK_SIZE/10,PROD_STK_SIZE,0,0,0,
                 OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,&err);

    // Task đọc BTN
    static OS_TCB  tcb_btn;
    static CPU_STK stk_btn[512];
    OSTaskCreate(&tcb_btn, "BtnPoll", BtnTask_Poll, 0, 8u,
                 stk_btn, 512/10, 512, 0, 0, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, &err);

    xil_printf("[Init] Done. MODE_0 mặc định (BTN0 toggle để đổi)\r\n");
}

// =============================
// Task poll nút bấm
// =============================
static void BtnTask_Poll(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;
    uint8_t prev = 0, curr = 0;
    uint32_t debounce = 0;

    xil_printf("[BTN] Polling task start\r\n");

    for (;;) {
        curr = btn0_read();

        if (curr && !prev && debounce == 0) { // vừa bấm
            g_mode ^= 1u;
            xil_printf("[BTN] BTN0 pressed -> toggle MODE_%u\r\n", g_mode);
            debounce = 5; // chờ 5*50ms = 250ms tránh dội
        }

        prev = curr;
        if (debounce > 0) debounce--;

        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

// =============================
// Task1: Producer A
// =============================
static void Task1_Producer(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;
    uint32_t a;

    for (;;) {
        a = rand() % 301;
        OSQPost(&g_q_data, (void*)(uintptr_t)a, 0u, OS_OPT_POST_FIFO, &err);
        xil_printf("[T1] produced A=%lu\r\n", a);
        OSSemPost(&g_sem_data, OS_OPT_POST_1, &err);
        OSTimeDlyHMSM(0, 0, 0, PROD_T1_PERIOD_MS, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

// =============================
// Task3: Producer B
// =============================
static void Task3_Producer(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;

    for (;;) {
        uint32_t b = next_wave();
        OSQPost(&g_q_alt, (void*)(uintptr_t)b, 0u, OS_OPT_POST_FIFO, &err);
        xil_printf("[T3] produced B=%lu\r\n", b);
        OSSemPost(&g_sem_alt, OS_OPT_POST_1, &err);
        OSTimeDlyHMSM(0, 0, 0, PROD_T3_PERIOD_MS, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

/*
// =============================
// Task2: Consumer
// =============================
static void Task2_Consumer(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;
    OS_MSG_SIZE msg_size;
    uint8_t last_mode = 255;

    xil_printf("[T2] start\r\n");

    for (;;) {
        if (g_mode != last_mode) {
            xil_printf("[T2] MODE_%u active\r\n", g_mode);
            last_mode = g_mode;
        }

        if (g_mode == 0u) {
            OSSemPend(&g_sem_data, 0, OS_OPT_PEND_BLOCKING, 0, &err);
            if (err != OS_ERR_NONE) continue;
            void *p_msg = OSQPend(&g_q_data, 0, OS_OPT_PEND_BLOCKING, &msg_size, 0, &err);
            if (err == OS_ERR_NONE)
                xil_printf("[T2] mode=0 consume A=%lu\r\n", (uint32_t)(uintptr_t)p_msg);
        } else {
            OSSemPend(&g_sem_alt, 0, OS_OPT_PEND_BLOCKING, 0, &err);
            if (err != OS_ERR_NONE) continue;
            void *p_msg = OSQPend(&g_q_alt, 0, OS_OPT_PEND_BLOCKING, &msg_size, 0, &err);
            if (err == OS_ERR_NONE)
                xil_printf("[T2] mode=1 consume B=%lu\r\n", (uint32_t)(uintptr_t)p_msg);
        }

        OSTimeDlyHMSM(0, 0, 0, PROD_T2_WORK_MS, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}
*/

// =============================
// Task2: Consumer (flush backlog khi đổi mode)
// =============================
static void Task2_Consumer(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;
    OS_MSG_SIZE msg_size;
    uint8_t last_mode = 255;

    xil_printf("[T2] start\r\n");

    for (;;) {
        if (g_mode != last_mode) {
            xil_printf("[T2] MODE change: %u -> %u (flush backlog)\r\n",
                       (unsigned)last_mode, (unsigned)g_mode);
            if (g_mode == 0u) flush_src(&g_q_data, &g_sem_data);
            else              flush_src(&g_q_alt,  &g_sem_alt);
            last_mode = g_mode;
        }

        if (g_mode == 0u) {
            OSSemPend(&g_sem_data, 0, OS_OPT_PEND_BLOCKING, 0, &err);
            if (err != OS_ERR_NONE) continue;
            void *p_msg = OSQPend(&g_q_data, 0, OS_OPT_PEND_BLOCKING, &msg_size, 0, &err);
            if (err == OS_ERR_NONE)
                xil_printf("[T2] mode=0 consume A=%lu\r\n", (uint32_t)(uintptr_t)p_msg);
        } else {
            OSSemPend(&g_sem_alt, 0, OS_OPT_PEND_BLOCKING, 0, &err);
            if (err != OS_ERR_NONE) continue;
            void *p_msg = OSQPend(&g_q_alt, 0, OS_OPT_PEND_BLOCKING, &msg_size, 0, &err);
            if (err == OS_ERR_NONE)
                xil_printf("[T2] mode=1 consume B=%lu\r\n", (uint32_t)(uintptr_t)p_msg);
        }

        OSTimeDlyHMSM(0, 0, 0, PROD_T2_WORK_MS, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

// =============================
// Wave giả lập cho Task3
// =============================
static inline uint32_t next_wave(void)
{
    static uint32_t t = 0;
    t += 7;
    return (t & 0xFFu);
}
