// osal_ucos3.c
#include "osal.h"
#include <Source/os.h>
#include <string.h>
#include "xscugic.h"

static OSAL_Config g_cfg;

int OSAL_Init(const OSAL_Config *cfg) {
    if (!cfg) return -1;
    g_cfg = *cfg;
    return 0;
}
OSAL_LogFn OSAL_Log(void){ return g_cfg.log ? g_cfg.log : 0; }
void*      OSAL_Platform(void){ return g_cfg.platform_ctx; }

void OSAL_SleepMs(uint32_t ms){
    OS_ERR err;
    OSTimeDlyHMSM(0,0, ms/1000, ms%1000, OS_OPT_TIME_HMSM_STRICT, &err);
    (void)err;
}

// --- Sem ---
struct OSAL_Sem { OS_SEM obj; };
int OSAL_SemCreate(OSAL_Sem **out, uint32_t initial){
    static OSAL_Sem pool; // demo: 1 sem là đủ
    OS_ERR err; 
    OSSemCreate(&pool.obj, "sem", initial, &err);
    if (err != OS_ERR_NONE) {
        return -1;
    }
    *out = &pool; 
    return 0;
}
int OSAL_SemPend(OSAL_Sem *s, uint32_t to_ms){
    OS_ERR err;
    if (to_ms==0) OSSemPend(&s->obj, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    else          OSSemPend(&s->obj, to_ms, OS_OPT_TIME_TIMEOUT, 0, &err);
    return (err==OS_ERR_NONE)?0:-1;
}
int OSAL_SemPost(OSAL_Sem *s){
    OS_ERR err; OSSemPost(&s->obj, OS_OPT_POST_1, &err);
    return (err==OS_ERR_NONE)?0:-1;
}

// --- Task ---
struct OSAL_Task { OS_TCB tcb; };
int OSAL_TaskCreate(OSAL_Task **out, const char *name, OSAL_TaskFn fn,
                    void *arg, uint32_t stk_words, uint32_t prio){
    static OSAL_Task t; static CPU_STK stk[1024]; // demo: 1 task + stack tĩnh
    if (stk_words==0 || stk_words>1024) stk_words = 1024;
    OS_ERR err;
    OSTaskCreate(&t.tcb, (CPU_CHAR*)name, fn, arg, prio,
                 &stk[0], stk_words/10, stk_words, 0u, 0u, 0,
                 OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, &err);
    if (err!=OS_ERR_NONE) return -1;
    *out = &t; return 0;
}

// --- IRQ (dựa GIC) ---
void OSAL_EnterIsr(void){ OSIntEnter(); }
void OSAL_ExitIsr (void){ OSIntExit();  }

int  OSAL_IrqConnect(int irq_id, OSAL_IrqHandler handler, void *arg,
                     uint8_t prio, uint8_t trig){
    XScuGic *gic = (XScuGic*)OSAL_Platform();
    if (!gic) return -1;
    XScuGic_SetPriorityTriggerType(gic, irq_id, prio, trig);
    int st = XScuGic_Connect(gic, irq_id, (Xil_ExceptionHandler)handler, arg);
    if (st != XST_SUCCESS) return -1;

    // Bật handler top-level của GIC & CPU nếu cần
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
        (Xil_ExceptionHandler)XScuGic_InterruptHandler, gic);
    Xil_ExceptionEnable();

    return 0;
}
void OSAL_IrqEnable (int irq_id){
    XScuGic *gic = (XScuGic*)OSAL_Platform();
    if (gic) XScuGic_Enable(gic, irq_id);
}
void OSAL_IrqDisable(int irq_id){
    XScuGic *gic = (XScuGic*)OSAL_Platform();
    if (gic) XScuGic_Disable(gic, irq_id);
}
