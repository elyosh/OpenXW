#ifndef XW_RUNTIME_DOS94_ASSETS_H
#define XW_RUNTIME_DOS94_ASSETS_H

#include "xw_dos94/assets/tables.h"
#include "xw_runtime/storage/storage.h"

/* Borrowed, bounded reference. A generation prevents reuse after mission cleanup. */
typedef struct Dos94ByteView {
	uint32_t generation;
	uint16_t allocation;
	size_t offset, size;
} Dos94ByteView;

typedef struct Dos94ModelView Dos94ModelView;

typedef struct Dos94Model {
	Dos94ModelMetadata metadata;
	Dos94ByteView payload, alternateBitmap;
	Dos94ModelView* decoded;
	char resourceType[5], resourceName[9];
} Dos94Model;

/* Single mission owner, on the Landru task thread. Aliases copy views, never ownership. */
void Dos94Assets_Select(XwGameVersion version);
XwGameVersion Dos94Assets_Version(void);
const Dos94ModelMetadata* Dos94Assets_Template(uint16_t model);
/* Reset mutable metadata from the selected revision's immutable catalogue. */
void Dos94Assets_Reset(void);
/* Release payloads and derived views while retaining authoritative mission metadata. */
void Dos94Assets_ReleaseModels(void);
Dos94Model* Dos94Assets_Model(uint16_t model);
bool Dos94Assets_Allocate(size_t size, Dos94ByteView* out);
bool Dos94Assets_Read(AeronFile* file, size_t size, Dos94ByteView* out);
bool Dos94Assets_Subview(Dos94ByteView view, size_t offset, size_t size, Dos94ByteView* out);
uint8_t* Dos94Assets_Data(Dos94ByteView view);
bool Dos94Assets_Alias(uint16_t model, uint16_t source);
/* Only an absent DOS94 archive permits OLSPECS; DOS93 always uses its own archive. */
AeronFile* Dos94Assets_OpenSpecies(void);

#endif
