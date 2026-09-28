#ifndef XW_LANDRU_CONFIG_H
#define XW_LANDRU_CONFIG_H

#if defined(LANDRU_GAME_TIE)
#error X-Wing must select its Landru version before including Landru headers
#endif

#ifndef LANDRU_GAME_XW
#define LANDRU_GAME_XW
#endif

#ifdef XW_MODERN
#if defined(LANDRU_CONFIG_H) && !defined(LANDRU_MODERN)
#error Landru modern extensions must be selected before including Landru headers
#endif
#ifndef LANDRU_MODERN
#define LANDRU_MODERN
#endif
#else
#if defined(LANDRU_CONFIG_H) && defined(LANDRU_MODERN)
#error Matching layout must be selected before including Landru headers
#endif
/* Independent matching checks can inherit native CMake definitions. */
#undef LANDRU_MODERN
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <landru/config.h>

#ifdef __cplusplus
}
#endif

#endif
