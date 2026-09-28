#ifndef XW_DOS94_ASSETS_MODELS_H
#define XW_DOS94_ASSETS_MODELS_H

#include "xw_runtime/storage/dos94_assets.h"

typedef enum Dos94MeshLayout { DOS94_MESH_CPLX, DOS94_MESH_CRFT, DOS94_MESH_SIMPLE } Dos94MeshLayout;

/* Offsets retain the DOS payload base, including self-relative references. */
typedef struct Dos94MeshView {
	Dos94ByteView payload;
	Dos94MeshLayout layout;
	uint16_t offset, vertices, normals, faces, streams, faceBsp, markings;
	uint8_t format, color, vertexCount, edgeCount, faceCount, coordinateShift;
	int16_t bounds[6];
} Dos94MeshView;

typedef struct Dos94Lod {
	int32_t maxDepth;
	uint16_t recordOffset;
	Dos94MeshView mesh;
} Dos94Lod;

typedef struct Dos94ComponentView {
	uint16_t firstLod, lodCount;
} Dos94ComponentView;

struct Dos94ModelView {
	Dos94ByteView payload;
	Dos94MeshLayout layout;
	uint16_t componentCount;
	uint8_t bspNodeCount;
	uint16_t imageCount, lodCount;
	Dos94ComponentView* components;
	Dos94Lod* lods;
	uint16_t* imageOffsets;
};

typedef struct Dos94FaceView {
	int16_t normal[3];
	/* CRFT uses seven count bits; callers never interpret the packed source flags. */
	uint8_t color, vertexCount;
	bool twoSided, gouraud;
	Dos94ByteView stream;
} Dos94FaceView;

typedef struct Dos94ComponentNode {
	int16_t normal[3], point[3];
	uint16_t firstChildOffset, secondChildOffsetOrComponent;
} Dos94ComponentNode;

bool Dos94Models_Decode(Dos94Model* model);
bool Dos94Models_DecodeLods(Dos94ByteView payload, uint16_t offset, Dos94MeshLayout layout, Dos94Lod** out,
							uint16_t* count);
bool Dos94Models_DecodeMesh(Dos94ByteView payload, uint16_t offset, Dos94MeshLayout layout,
							Dos94MeshView* out);
const Dos94Lod* Dos94Models_ComponentLods(uint16_t model, uint16_t component, uint16_t* count);
bool Dos94Models_ComponentNode(uint16_t model, uint16_t node, Dos94ComponentNode* out);
bool Dos94Models_Bitmap(uint16_t model, uint16_t image, Dos94ByteView* out);
bool Dos94Models_Vertex(const Dos94MeshView* mesh, uint16_t vertex, int16_t out[3]);
bool Dos94Models_Face(const Dos94MeshView* mesh, uint16_t face, Dos94FaceView* out);
bool Dos94Models_ReadWord(Dos94ByteView view, size_t offset, uint16_t* out);
bool Dos94Models_ReadDword(Dos94ByteView view, size_t offset, uint32_t* out);

/* Load once per mission; subsequent requirements acquire only missing payloads. */
bool Dos94_fediskio_loadspecies(bool surface, bool provingGrounds, char* error, size_t capacity);
bool Dos94Models_Ensure(uint16_t model, char* error, size_t capacity);
void Dos94Models_CloseCatalog(void);
const Dos94Lod* Dos94Models_ProjectileLods(uint16_t model, uint16_t* count);
bool Dos94Models_Hyperstar(Dos94MeshView* out);
void Dos94_create_initcomponents(CraftData* craft, uint16_t objectType);
uint16_t Dos94_corvetteguncomponent(bool second);
void Dos94_create_initclosedfoils(CraftData* craft, uint16_t objectType);

#endif
