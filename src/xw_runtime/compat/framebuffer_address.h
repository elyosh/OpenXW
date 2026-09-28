#ifndef XW_RUNTIME_COMPAT_FRAMEBUFFER_ADDRESS_H
#define XW_RUNTIME_COMPAT_FRAMEBUFFER_ADDRESS_H

#include <stdint.h>

enum { XW_FRAMEBUFFER_LEGACY_BASE_ADDRESS = 0xA0000 };

/* The RGB rasterizer rounds the framebuffer base down to a pixel boundary. */
static __inline uint8_t* XwFramebufferAddress_Align16BitBase(uint8_t* base) {
	return (uint8_t*)((uintptr_t)base / sizeof(uint16_t) * sizeof(uint16_t));
}

#ifdef XW_MODERN
/* Modern surfaces are linear; the physical VGA aperture does not apply. */
static __inline int XwFramebufferAddress_IsLegacyBase(const uint8_t* base) {
	(void)base;
	return 0;
}
#else
static __inline int XwFramebufferAddress_IsLegacyBase(const uint8_t* base) {
	return base == (const uint8_t*)(uintptr_t)XW_FRAMEBUFFER_LEGACY_BASE_ADDRESS;
}
#endif

#endif
