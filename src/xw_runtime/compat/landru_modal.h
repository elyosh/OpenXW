#ifndef XW_RUNTIME_COMPAT_LANDRU_MODAL_H
#define XW_RUNTIME_COMPAT_LANDRU_MODAL_H

#include <stdint.h>

/* Original binary entry points, replaced by shared Landru tasks in the port. */
#ifndef XW_MODERN
int16_t xviewadd_Handle_View(void);
void xcanvas_Fade_Screen_To_Video(int16_t isDialog);
#endif

#endif
