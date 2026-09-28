#ifndef XW_RUNTIME_OPT_NATIVE_H
#define XW_RUNTIME_OPT_NATIVE_H
#include "xw/assets/opt_model.h"
#include <aeron/vfs.h>

#ifdef __cplusplus
extern "C" {
#endif
void XwOpt_Relocate(OptimizedPolyObject* model);
void XwOpt_RelocateNode(OptNode* node, intptr_t delta);
/* Read takes ownership of the open VFS stream. */
uint16_t XwOpt_Read(AeronFile* file, const char* label, int* version, unsigned int* native_size);
uint16_t XwOpt_Load(const char* path, int* version, unsigned int* native_size);
/* Native contiguous model blocks are aligned to the widest pointer field. */
size_t XwOpt_AlignSize(size_t size);
uint8_t* XwOpt_AlignPointer(uint8_t* pointer);
#ifdef __cplusplus
}
#endif

#endif
