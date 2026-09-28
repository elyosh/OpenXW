#ifndef XW_RUNTIME_COMPAT_POINTER_HASH_H
#define XW_RUNTIME_COMPAT_POINTER_HASH_H

#include <stddef.h>
#include <stdint.h>

/* Hash native pointer bits without narrowing or changing the stored key. */
static __inline size_t XwPointerHash_LowBits(const void* key, size_t mask) { return (uintptr_t)key & mask; }

#endif
