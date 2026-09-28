#ifndef XW_VIDEO_OPTIONS_H
#define XW_VIDEO_OPTIONS_H
#include "xw_runtime/config/settings.h"
/* Host output only. A failed change restores the previous host settings. */
bool XwVideoOptions_Apply(const XwSettings* previous, const XwSettings* requested, char* error,
						  size_t capacity);
#endif
