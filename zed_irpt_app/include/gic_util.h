#ifndef GIC_UTIL_H
#define GIC_UTIL_H

#include "xscugic.h"

// Lấy GIC instance đã init bởi UCOS (nếu có) hoặc fallback an toàn
XScuGic* GIC_GetInstance(void);

#endif // GIC_UTIL_H
