#include "osal_task.h"
#include "osal.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#ifndef OSAL_MAX_TASKS
#define OSAL_MAX_TASKS 16
#endif

typedef struct {
    uint8_t       used;
    TaskHandle_t  h;
    const char*   name;
} _TaskSlot;

static _TaskSlot s_tasks[OSAL_MAX_TASKS];

static int _alloc_slot(void){
    for (int i=0;i<OSAL_MAX_TASKS;i++) if(!s_tasks[i].used) return i;
    return -1;
}
static int _find_slot(OSAL_TaskHandle h){
    for (int i=0;i<OSAL_MAX_TASKS;i++) if(s_tasks[i].used && (OSAL_TaskHandle)s_tasks[i].h==h) return i;
    return -1;
}

/* ===== Core API ===== */
OSAL_Status OSAL_TaskCreate(OSAL_TaskHandle* h,
                            OSAL_TaskEntry entry,
                            void* arg,
                            const OSAL_TaskAttr* a)
{
    if (!g_osal.initialized || !h || !entry || !a) return OSAL_EINVAL;

    UBaseType_t stack_words = (UBaseType_t)(a->stack_size / sizeof(StackType_t));
    if (stack_words < configMINIMAL_STACK_SIZE) stack_words = configMINIMAL_STACK_SIZE;

    TaskHandle_t th = NULL;
    BaseType_t r = xTaskCreate((TaskFunction_t)entry,
                               (a->name ? a->name : "task"),
                               stack_words,
                               arg,
                               (UBaseType_t)a->prio,   // FreeRTOS: BIG number = HIGH priority
                               &th);
    if (r != pdPASS) return OSAL_EOS;

    int idx = _alloc_slot();
    if (idx < 0) { vTaskDelete(th); return OSAL_EOS; }
    s_tasks[idx].used = 1;
    s_tasks[idx].h    = th;
    s_tasks[idx].name = a->name;

    *h = (OSAL_TaskHandle)th;
    OSAL_LOG("[OSAL] TaskCreate: %s prio=%u\r\n", (a->name?a->name:"task"), (unsigned)a->prio);
    return OSAL_OK;
}

OSAL_Status OSAL_TaskDelete(OSAL_TaskHandle h)
{
    TaskHandle_t th = (TaskHandle_t)h;
    if (th == NULL) { vTaskDelete(NULL); return OSAL_OK; } // delete self

    int idx = _find_slot(h);
    if (idx >= 0) s_tasks[idx].used = 0;

    vTaskDelete(th);
    return OSAL_OK;
}

OSAL_Status OSAL_TaskSuspend(OSAL_TaskHandle h)
{
    TaskHandle_t th = (TaskHandle_t)h;
    if (th == NULL) th = NULL; // suspend self
    vTaskSuspend(th);
    return OSAL_OK;
}

OSAL_Status OSAL_TaskResume(OSAL_TaskHandle h)
{
    if (!h) return OSAL_EINVAL;
    vTaskResume((TaskHandle_t)h);
    return OSAL_OK;
}

OSAL_Status OSAL_TaskChangePrio(OSAL_TaskHandle h, uint8_t new_prio)
{
    if (!h) return OSAL_EINVAL;
    vTaskPrioritySet((TaskHandle_t)h, (UBaseType_t)new_prio);
    return OSAL_OK;
}

OSAL_Status OSAL_TaskGetState(OSAL_TaskHandle h, OSAL_TaskState* state)
{
    if (!state) return OSAL_EINVAL;
    if (!h) { *state = OSAL_TASK_STATE_INVALID; return OSAL_EINVAL; }

    eTaskState st = eTaskGetState((TaskHandle_t)h);
    switch (st) {
        case eRunning:   *state = OSAL_TASK_STATE_RUNNING;  break;
        case eReady:     *state = OSAL_TASK_STATE_READY;    break;
        case eBlocked:   *state = OSAL_TASK_STATE_WAITING;  break;
        case eSuspended: *state = OSAL_TASK_STATE_SUSPENDED;break;
        case eDeleted:   *state = OSAL_TASK_STATE_COMPLETED;break;
        default:         *state = OSAL_TASK_STATE_INVALID;  break;
    }
    return OSAL_OK;
}

OSAL_Status OSAL_TaskGetName(OSAL_TaskHandle h, const char** name)
{
    if (!name) return OSAL_EINVAL;
    const char* n = pcTaskGetName((TaskHandle_t)h);
    *name = n ? n : "(unnamed)";
    return OSAL_OK;
}

/* ===== Utility ===== */
uint32_t OSAL_TaskCount(void)
{
    uint32_t c=0;
    for (int i=0;i<OSAL_MAX_TASKS;i++) if (s_tasks[i].used) c++;
    return c;
}

OSAL_Status OSAL_TaskForEach(void (*cb)(OSAL_TaskHandle h, void* arg), void* arg)
{
    if (!cb) return OSAL_EINVAL;
    for (int i=0;i<OSAL_MAX_TASKS;i++)
        if (s_tasks[i].used)
            cb((OSAL_TaskHandle)s_tasks[i].h, arg);
    return OSAL_OK;
}

/* ===== Time/Yield ===== */
void OSAL_TaskDelayMs(uint32_t ms)
{
    TickType_t t = pdMS_TO_TICKS(ms);
    if (t == 0) t = 1;
    vTaskDelay(t);
}

void OSAL_TaskYield(void)
{
    taskYIELD();
}
