#ifndef XW_RUNTIME_COMPAT_MIDDLEWARE_CRT_H
#define XW_RUNTIME_COMPAT_MIDDLEWARE_CRT_H

#include <string.h>

#ifdef XW_MODERN
#ifdef _WIN32
#define XwStrCaseCmp _stricmp
#else
#include <strings.h>
#define XwStrCaseCmp strcasecmp
#endif
#else
#define XwStrCaseCmp _strcmpi
#endif

#endif
