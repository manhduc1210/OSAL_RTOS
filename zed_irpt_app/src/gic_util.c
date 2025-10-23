#include "gic_util.h"
#include "xparameters.h"
#include <string.h>

extern XScuGic *UCOS_Int_InitGIC(void) __attribute__((weak));

XScuGic* GIC_GetInstance(void)
{
    // 1) Nếu UCOS export sẵn GIC instance
    if (UCOS_Int_InitGIC) {
        XScuGic *p = UCOS_Int_InitGIC();
        if (p) return p;
    }

    // 2) Fallback: dựng instance tối thiểu (không CfgInitialize)
    static XScuGic GicLocal;
    static int     inited = 0;

    if (!inited) {
        XScuGic_Config *cfg = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
        if (!cfg) return NULL;
        memset(&GicLocal, 0, sizeof(GicLocal));
        GicLocal.Config  = cfg;                    // gán con trỏ config
        GicLocal.IsReady = XIL_COMPONENT_IS_READY; // đánh dấu sẵn sàng
        inited = 1;
    }
    return &GicLocal;
}
