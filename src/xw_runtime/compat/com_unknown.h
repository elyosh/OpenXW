#ifndef XW_RUNTIME_COMPAT_COM_UNKNOWN_H
#define XW_RUNTIME_COMPAT_COM_UNKNOWN_H

#include <aeron/compat/win_types.h>

/* Base COM interface, using the same native pointers and calling convention
 * as Aeron's DirectX interfaces. */
typedef struct IUnknown IUnknown;

typedef struct IUnknownVtbl {
	HRESULT(AERON_DXAPI* QueryInterface)(IUnknown*, DxRefIid, void**);
	uint32_t(AERON_DXAPI* AddRef)(IUnknown*);
	uint32_t(AERON_DXAPI* Release)(IUnknown*);
} IUnknownVtbl;

struct IUnknown {
	const IUnknownVtbl* lpVtbl;
};

#endif
