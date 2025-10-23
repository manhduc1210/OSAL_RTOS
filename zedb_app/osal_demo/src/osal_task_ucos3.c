#include "osal_task.h"
#include "osal.h"

// Micrium headers
#include <Source/os.h>     // uC/OS-III core
#include <stdlib.h>

#ifndef OSAL_MAX_TASKS
#define OSAL_MAX_TASKS 8
#endif

typedef struct {
    uint8_t   used;
    OS_TCB    tcb;
    CPU_STK  *stk;
    uint32_t  stk_words; // số phần tử CPU_STK
} _TaskSlot;

static _TaskSlot s_tasks[OSAL_MAX_TASKS];

/* ===== Internal helpers ===== */
static int _find_slot(OSAL_TaskHandle h) {
    for (int i=0; i<OSAL_MAX_TASKS; ++i)
        if (s_tasks[i].used && (OSAL_TaskHandle)&s_tasks[i].tcb == h)
            return i;
    return -1;
}

static int _alloc_slot(void) {
    for (int i=0; i<OSAL_MAX_TASKS; ++i)
        if (!s_tasks[i].used) return i;
    return -1;
}

/* ===== API Implementation ===== */

OSAL_Status OSAL_TaskCreate(OSAL_TaskHandle* h, OSAL_TaskEntry entry, void* arg, const OSAL_TaskAttr* a) {
    if (!g_osal.initialized || !h || !entry || !a || a->stack_size < 256) return OSAL_EINVAL;

    int idx = _alloc_slot();
    if (idx < 0) return OSAL_EOS;

    uint32_t stk_words = a->stack_size / sizeof(CPU_STK);
    s_tasks[idx].stk = (CPU_STK*)malloc(sizeof(CPU_STK) * stk_words);
    if (!s_tasks[idx].stk) return OSAL_EOS;

    s_tasks[idx].used = 1;
    s_tasks[idx].stk_words = stk_words;

    OS_ERR err;
    OSTaskCreate(&s_tasks[idx].tcb,
                 (CPU_CHAR*)(a->name ? a->name : "task"),
                 (OS_TASK_PTR)entry,
                 arg,
                 a->prio,
                 s_tasks[idx].stk,
                 stk_words / 10,                // low watermark
                 stk_words,
                 0u,
                 0u,
                 0u,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);
    if (err != OS_ERR_NONE) {
        free(s_tasks[idx].stk);
        s_tasks[idx].used = 0;
        return OSAL_EOS;
    }
    *h = (OSAL_TaskHandle)&s_tasks[idx].tcb;
    return OSAL_OK;
}

OSAL_Status OSAL_TaskDelete(OSAL_TaskHandle h)
{
    int idx = _find_slot(h);
    if (idx < 0) return OSAL_EINVAL;

    OS_ERR err;
    OSTaskDel((OS_TCB*)h, &err);
    if (err != OS_ERR_NONE) return OSAL_EOS;

    free(s_tasks[idx].stk);
    memset(&s_tasks[idx], 0, sizeof(_TaskSlot));
    return OSAL_OK;
}

OSAL_Status OSAL_TaskSuspend(OSAL_TaskHandle h)
{
    if (!h) return OSAL_EINVAL;
    OS_ERR e;
    OSTaskSuspend((OS_TCB*)h, &e);
    return (e == OS_ERR_NONE) ? OSAL_OK : OSAL_EOS;
}

OSAL_Status OSAL_TaskResume(OSAL_TaskHandle h)
{
    if (!h) return OSAL_EINVAL;
    OS_ERR e;
    OSTaskResume((OS_TCB*)h, &e);
    return (e == OS_ERR_NONE) ? OSAL_OK : OSAL_EOS;
}

OSAL_Status OSAL_TaskChangePrio(OSAL_TaskHandle h, uint8_t new_prio)
{
    if (!h) return OSAL_EINVAL;
    OS_ERR e;
    OSTaskChangePrio((OS_TCB*)h, new_prio, &e);
    return (e == OS_ERR_NONE) ? OSAL_OK : OSAL_EOS;
}

OSAL_Status OSAL_TaskGetState(OSAL_TaskHandle h, OSAL_TaskState* state)
{
    if (!h || !state) return OSAL_EINVAL;
    OS_TCB* tcb = (OS_TCB*)h;

    switch (tcb->TaskState) {
        case OS_TASK_STATE_RDY:     *state = OSAL_TASK_STATE_READY; break;
        case OS_TASK_STATE_DLY:
        case OS_TASK_STATE_PEND:
        case OS_TASK_STATE_PEND_TIMEOUT: *state = OSAL_TASK_STATE_WAITING; break;
        case OS_TASK_STATE_SUSPENDED: *state = OSAL_TASK_STATE_SUSPENDED; break;
        default: *state = OSAL_TASK_STATE_INVALID; break;
    }
    return OSAL_OK;
}

OSAL_Status OSAL_TaskGetName(OSAL_TaskHandle h, const char** name)
{
    if (!h || !name) return OSAL_EINVAL;
    OS_TCB* tcb = (OS_TCB*)h;
    *name = tcb->NamePtr ? tcb->NamePtr : "(unnamed)";
    return OSAL_OK;
}

uint32_t OSAL_TaskCount(void)
{
    uint32_t count = 0;
    for (int i=0; i<OSAL_MAX_TASKS; ++i)
        if (s_tasks[i].used)
            count++;
    return count;
}

OSAL_Status OSAL_TaskForEach(void (*cb)(OSAL_TaskHandle h, void* arg), void* arg)
{
    if (!cb) return OSAL_EINVAL;
    for (int i=0; i<OSAL_MAX_TASKS; ++i)
        if (s_tasks[i].used)
            cb((OSAL_TaskHandle)&s_tasks[i].tcb, arg);
    return OSAL_OK;
}

void OSAL_TaskYield(void) {
    OS_ERR err;
    OSTimeDly(1u, OS_OPT_TIME_DLY, &err); // nhường CPU
}

void OSAL_TaskDelayMs(uint32_t ms)
{
    OS_ERR err;
    OSTimeDlyHMSM(0, 0, (CPU_INT16U)(ms / 1000u), (CPU_INT16U)(ms % 1000u), OS_OPT_TIME_HMSM_STRICT, &err);
    // OSTimeDlyHMSM(0, 0, (CPU_INT16U)(ms / 1000u), 0, OS_OPT_TIME_HMSM_STRICT, &err);
}
