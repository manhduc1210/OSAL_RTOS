// osal.h
#pragma once
#include <stdint.h>

typedef enum {
    OSAL_BACKEND_UCOS3 = 1,
    // (maybe add FREERTOS/LINUX later`)
} OSAL_Backend;

typedef void (*OSAL_LogFn)(const char *fmt, ...);

typedef struct {
    OSAL_Backend backend;
    OSAL_LogFn   log;           // vd: xil_printf
    void*        platform_ctx;  // vd: (XScuGic*) từ GIC_GetInstance()
} OSAL_Config;

typedef struct OSAL_Sem OSAL_Sem;
typedef struct OSAL_Task OSAL_Task;

typedef void (*OSAL_TaskFn)(void *arg);
typedef void (*OSAL_IrqHandler)(void *arg);

int  OSAL_Init   (const OSAL_Config *cfg);
void OSAL_SleepMs(uint32_t ms);
OSAL_LogFn OSAL_Log(void);
void*      OSAL_Platform(void);

// ---- Semaphore ----
int  OSAL_SemCreate (OSAL_Sem **out, uint32_t initial);
int  OSAL_SemPend   (OSAL_Sem *sem, uint32_t timeout_ms); // 0 = block
int  OSAL_SemPost   (OSAL_Sem *sem);

// ---- Task ----
int  OSAL_TaskCreate(OSAL_Task **out, const char *name, OSAL_TaskFn fn,
                     void *arg, uint32_t stack_words, uint32_t prio);

// ---- IRQ (GIC) ----
int  OSAL_IrqConnect (int irq_id, OSAL_IrqHandler handler, void *arg,
                      uint8_t prio, uint8_t trig); // trig: 3 = LEVEL_HIGH
void OSAL_IrqEnable  (int irq_id);
void OSAL_IrqDisable (int irq_id);

// ---- ISR section marks (cho backend cần) ----
void OSAL_EnterIsr(void);
void OSAL_ExitIsr (void);
