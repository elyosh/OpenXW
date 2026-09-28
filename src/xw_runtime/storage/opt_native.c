/* Native OPT loading/relocation adapted from OpenXvT. */

#include "xw_runtime/storage/opt_native.h"
#include "aeron/log.h"
#include "xw/util/memory.h"
#include "xw_runtime/snapshot/render_assets.h"
#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/storage.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define XW_OPT_LIMIT (128u * 1024u * 1024u)
#define XW_OPT_DEPTH 256

typedef struct XwOptEntry {
	uint32_t address;
	uint32_t name;
	uint32_t children;
	uint32_t payload;
	uint32_t palette;
	uint32_t embedded_palette;
	int32_t type, count, param;
	size_t node_offset, children_offset, texture_offset, payload_offset, payload_size, payload_copy_size,
		palette_offset;
	int visiting;
} XwOptEntry;

typedef struct XwOptPalette {
	uint32_t address;
	size_t offset;
} XwOptPalette;

typedef struct XwOptDecode {
	uint8_t* bytes;
	uint32_t size, base;
	XwOptEntry* nodes;
	size_t count, capacity, native_size;
	int failed;
	int version;
} XwOptDecode;

size_t XwOpt_AlignSize(size_t size) { return (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1); }

uint8_t* XwOpt_AlignPointer(uint8_t* pointer) {
	return (uint8_t*)((uintptr_t)(pointer + sizeof(void*) - 1) & ~(uintptr_t)(sizeof(void*) - 1));
}

static uint32_t XwOpt_U32(const void* memory) {
	const uint8_t* p = memory;
	return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static const uint8_t* XwOpt_Address(XwOptDecode* decode, uint32_t address, size_t size) {
	if (!address || address < decode->base || address - decode->base > decode->size ||
		size > decode->size - (address - decode->base)) {
		decode->failed = 1;
		return NULL;
	}
	return decode->bytes + (address - decode->base);
}

static int XwOpt_String(XwOptDecode* decode, uint32_t address) {
	const uint8_t* value = XwOpt_Address(decode, address, 1);
	if (!value || !memchr(value, 0, decode->size - (address - decode->base))) {
		decode->failed = 1;
		return 0;
	}
	return 1;
}

static size_t XwOpt_Reserve(XwOptDecode* decode, size_t size) {
	size_t offset = XwOpt_AlignSize(decode->native_size);
	if (size > XW_OPT_LIMIT || offset > XW_OPT_LIMIT - size) {
		decode->failed = 1;
		return 0;
	}
	decode->native_size = offset + size;
	return offset;
}

static int XwOpt_Index(const XwOptDecode* decode, uint32_t address) {
	for (size_t i = 0; i < decode->count; ++i)
		if (decode->nodes[i].address == address)
			return (int)i;
	return -1;
}

static int XwOpt_Texture(XwOptDecode* decode, XwOptEntry* entry) {
	const uint8_t* raw = XwOpt_Address(decode, entry->payload, 24);
	int64_t pixels, bytes, palettes;
	if (!raw)
		return 0;
	int32_t type = (int32_t)XwOpt_U32(raw + 4);
	int32_t texture_size = (int32_t)XwOpt_U32(raw + 8);
	int32_t data_size = (int32_t)XwOpt_U32(raw + 12);
	int32_t width = (int32_t)XwOpt_U32(raw + 16);
	int32_t height = (int32_t)XwOpt_U32(raw + 20);
	entry->palette = XwOpt_U32(raw);
	pixels = (int64_t)width * height;
	bytes = texture_size == pixels ? data_size : pixels;
	palettes = type ? (int64_t)type * 768 : entry->palette == entry->payload + 24 + bytes ? 12288 : 0;
	if (width <= 0 || height <= 0 || width > 16384 || height > 16384 || type < 0 || type > 16 ||
		bytes < pixels || bytes > XW_OPT_LIMIT || palettes > XW_OPT_LIMIT ||
		!XwOpt_Address(decode, entry->payload, (size_t)(24 + bytes + palettes)))
		return 0;
	if (!type && !XwOpt_Address(decode, entry->palette, 12288))
		return 0;
	entry->texture_offset = XwOpt_Reserve(decode, sizeof(OptTextureData) + (size_t)(bytes + palettes));
	if (palettes) {
		entry->embedded_palette = entry->payload + 24 + (uint32_t)bytes;
		entry->palette_offset = entry->texture_offset + sizeof(OptTextureData) + (size_t)bytes;
	}
	return !decode->failed;
}

static int XwOpt_ComparePalette(const void* lhs, const void* rhs) {
	const XwOptPalette* a = lhs;
	const XwOptPalette* b = rhs;
	return (a->address > b->address) - (a->address < b->address);
}

static int XwOpt_ResolvePalettes(XwOptDecode* decode) {
	size_t count = 0;
	for (size_t i = 0; i < decode->count; ++i)
		if (decode->nodes[i].embedded_palette)
			++count;
	XwOptPalette* palettes = count ? malloc(count * sizeof(*palettes)) : NULL;
	if (count && !palettes)
		return 0;
	size_t cursor = 0;
	for (size_t i = 0; i < decode->count; ++i) {
		const XwOptEntry* entry = &decode->nodes[i];
		if (entry->embedded_palette)
			palettes[cursor++] = (XwOptPalette) { entry->embedded_palette, entry->palette_offset };
	}
	if (count)
		qsort(palettes, count, sizeof(*palettes), XwOpt_ComparePalette);
	/* Resolve after visiting every node: shared palettes may precede their owner.
	 * Runtime fixup identifies that owner by its embedded palette's exact address. */
	int valid = 1;
	for (size_t i = 0; i < decode->count; ++i) {
		XwOptEntry* entry = &decode->nodes[i];
		if (entry->type != OPT_TEXTURE)
			continue;
		const uint8_t* raw = XwOpt_Address(decode, entry->payload, 24);
		if (XwOpt_U32(raw + 4) != 0) {
			if (entry->palette != entry->embedded_palette)
				entry->palette_offset = 0;
			continue;
		}
		XwOptPalette key = { entry->palette, 0 };
		const XwOptPalette* owner =
			count ? bsearch(&key, palettes, count, sizeof(*palettes), XwOpt_ComparePalette) : NULL;
		if (!owner) {
			valid = 0;
			break;
		}
		entry->palette_offset = owner->offset;
	}
	free(palettes);
	return valid;
}

static int XwOpt_Visit(XwOptDecode* decode, uint32_t address, unsigned depth, int* vertices, int* normals) {
	if (!address)
		return 1;
	int index = XwOpt_Index(decode, address);
	if (index >= 0)
		return !decode->nodes[index].visiting;
	const uint8_t* raw = XwOpt_Address(decode, address, 24);
	if (!raw || depth >= XW_OPT_DEPTH || decode->count >= 65536)
		return 0;
	if (decode->count == decode->capacity) {
		size_t capacity = decode->capacity ? decode->capacity * 2 : 64;
		XwOptEntry* grown = realloc(decode->nodes, capacity * sizeof(*grown));
		if (!grown)
			return 0;
		decode->nodes = grown;
		decode->capacity = capacity;
	}
	index = (int)decode->count++;
	XwOptEntry* entry = &decode->nodes[index];
	memset(entry, 0, sizeof(*entry));
	entry->address = address;
	entry->name = XwOpt_U32(raw);
	entry->type = (int32_t)XwOpt_U32(raw + 4);
	entry->count = (int32_t)XwOpt_U32(raw + 8);
	entry->children = XwOpt_U32(raw + 12);
	entry->param = (int32_t)XwOpt_U32(raw + 16);
	entry->payload = XwOpt_U32(raw + 20);
	entry->visiting = 1;
	/* The original default case preserves unrecognized nodes without a payload.
	 * State-only and unused (-1) nodes can retain stale allocation addresses. */
	int has_payload = entry->type >= 0 && entry->type <= OPT_MESHDESCRIPTOR && entry->type != 0 &&
					  entry->type != 8 && entry->type != OPT_NODE_TYPE_10 && entry->type != 12 &&
					  entry->type != 14 && entry->type != 18 && entry->type != OPT_NODE_TYPE_24;
	if (!has_payload)
		entry->payload = 0;
	if (entry->count < 0 || entry->count > 65536 ||
		(has_payload && (entry->param < 0 || entry->param > 1000000)) ||
		(entry->name && !XwOpt_String(decode, entry->name)))
		return 0;
	entry->node_offset = XwOpt_Reserve(decode, sizeof(OptNode));
	entry->children_offset = XwOpt_Reserve(decode, (size_t)entry->count * sizeof(OptNode*));
	if (entry->type == OPT_MESHVERTS)
		*vertices = entry->param;
	if (entry->type == OPT_VERTNORMALS)
		*normals = 1;
	if (entry->type == OPT_TEXTURE) {
		if (!XwOpt_Texture(decode, entry))
			return 0;
	} else if (entry->payload) {
		size_t size = 0;
		switch (entry->type) {
			case OPT_FACEDATA:
			case OPT_FACEDATA_15:
			case OPT_FACEDATA_16:
			case OPT_FACEDATA_17:
				size = 4 + (size_t)entry->param * (decode->version == 0 ? 84 : 100);
				if (!*normals)
					size += (size_t)*vertices * 12;
				break;
			case OPT_NODE_TYPE_2:
			case OPT_NODE_TYPE_23:
				size = 48;
				break;
			case OPT_MESHVERTS:
			case OPT_VERTNORMALS:
				size = (size_t)entry->param * 12;
				break;
			case OPT_NODE_TYPE_4:
			case OPT_NODE_TYPE_6:
			case OPT_NODE_TYPE_19:
				size = 12;
				break;
			case OPT_NODE_TYPE_5:
				size = 36;
				break;
			case OPT_NODEREF:
				if (!XwOpt_String(decode, entry->payload))
					return 0;
				size = strlen((const char*)XwOpt_Address(decode, entry->payload, 1)) + 1;
				break;
			case OPT_NODE_TYPE_9:
				size = (size_t)entry->param * 56;
				break;
			case OPT_TEXCOORDS:
				size = (size_t)entry->param * 8;
				break;
			case OPT_NODE_TYPE_21:
				size = (size_t)entry->param * 4;
				break;
			case OPT_NODE_TYPE_22:
				size = 16;
				break;
			case OPT_MESHDESCRIPTOR:
				size = 72;
				break;
			default:
				return 0;
		}
		if (!XwOpt_Address(decode, entry->payload, 0))
			return 0;
		size_t available = decode->size - (entry->payload - decode->base);
		/* The original can read beyond the serialized payload into its allocation.
		 * Copy available bytes and keep the native allocation's remainder zeroed. */
		entry->payload_copy_size = size < available ? size : available;
		entry->payload_size = size;
		entry->payload_offset = XwOpt_Reserve(decode, size);
	} else if (has_payload) {
		return 0;
	}
	int count = entry->count;
	const uint8_t* children = count ? XwOpt_Address(decode, entry->children, (size_t)count * 4) : NULL;
	if (count && !children)
		return 0;
	int child_vertices = *vertices, child_normals = *normals;
	for (int i = 0; i < count; ++i)
		if (!XwOpt_Visit(decode, XwOpt_U32(children + i * 4), depth + 1, &child_vertices, &child_normals))
			return 0;
	decode->nodes[index].visiting = 0;
	return !decode->failed;
}

static void* XwOpt_RawPointer(XwOptDecode* decode, uint8_t* raw_copy, uint32_t address) {
	return address ? raw_copy + (address - decode->base) : NULL;
}

static OptNode* XwOpt_NodePointer(XwOptDecode* decode, uint8_t* native, uint32_t address) {
	int index = XwOpt_Index(decode, address);
	return index < 0 ? NULL : (OptNode*)(native + decode->nodes[index].node_offset);
}

static void XwOpt_Expand(XwOptDecode* decode, uint8_t* native, uint8_t* raw_copy) {
	for (size_t i = 0; i < decode->count; ++i) {
		XwOptEntry* entry = &decode->nodes[i];
		OptNode* node = (OptNode*)(native + entry->node_offset);
		node->pName = XwOpt_RawPointer(decode, raw_copy, entry->name);
		node->nodeType = (OptNodeType)entry->type;
		node->childCount = entry->count;
		node->param1 = entry->param;
		node->param2 = entry->payload_size ? native + entry->payload_offset : NULL;
		if (entry->payload_copy_size)
			memcpy(node->param2, XwOpt_Address(decode, entry->payload, entry->payload_copy_size),
				   entry->payload_copy_size);
		node->pChildren = entry->count ? (OptNode**)(native + entry->children_offset) : NULL;
		const uint8_t* children =
			entry->count ? XwOpt_Address(decode, entry->children, (size_t)entry->count * 4) : NULL;
		for (int j = 0; j < entry->count; ++j)
			node->pChildren[j] = XwOpt_NodePointer(decode, native, XwOpt_U32(children + j * 4));
		if (entry->type == OPT_TEXTURE) {
			const uint8_t* raw = XwOpt_Address(decode, entry->payload, 24);
			OptTextureData* texture = (OptTextureData*)(native + entry->texture_offset);
			node->param2 = texture;
			texture->paletteType = (int32_t)XwOpt_U32(raw + 4);
			texture->textureSize = (int32_t)XwOpt_U32(raw + 8);
			texture->dataSize = (int32_t)XwOpt_U32(raw + 12);
			texture->width = (int32_t)XwOpt_U32(raw + 16);
			texture->height = (int32_t)XwOpt_U32(raw + 20);
			size_t bytes = (size_t)texture->width * texture->height;
			if (bytes == (size_t)texture->textureSize)
				bytes = (size_t)texture->dataSize;
			size_t palette_bytes = texture->paletteType ? (size_t)texture->paletteType * 768
								   : entry->palette == entry->payload + 24 + bytes ? 12288
																				   : 0;
			memcpy(texture + 1, raw + 24, bytes + palette_bytes);
			texture->palette = entry->palette_offset ? (uint16_t*)(native + entry->palette_offset) : NULL;
		}
	}
}

uint16_t XwOpt_Read(AeronFile* file, const char* path, int* version, unsigned int* native_size) {
	XwOptDecode decode = { 0 };
	uint8_t header[8];
	uint16_t handle = 0;
	*native_size = 0;
	*version = -1;
	if (!file)
		return 0;
	if (XwFile_Read(header, 1, 4, file) != 4)
		goto done;
	int32_t marker = (int32_t)XwOpt_U32(header);
	*version = marker > 0 ? 0 : marker == -1 ? 1 : marker == -2 ? 2 : -1;
	if (*version < 0)
		goto done;
	if (marker <= 0 && XwFile_Read(header, 1, 4, file) != 4)
		goto done;
	decode.size = XwOpt_U32(header);
	if (decode.size < 14 || decode.size > XW_OPT_LIMIT ||
		AeronVfs_GetSize(file) - AeronVfs_Tell(file) != decode.size)
		goto done;
	decode.bytes = malloc(decode.size);
	if (!decode.bytes || XwFile_Read(decode.bytes, 1, decode.size, file) != decode.size)
		goto done;
	decode.base = XwOpt_U32(decode.bytes);
	decode.version = *version;
	uint32_t roots = XwOpt_U32(decode.bytes + 6), root_address = XwOpt_U32(decode.bytes + 10);
	if (roots > 65536 || decode.base > UINT32_MAX - decode.size)
		goto done;
	const uint8_t* table = roots ? XwOpt_Address(&decode, root_address, (size_t)roots * 4) : NULL;
	if (roots && !table)
		goto done;
	decode.native_size = sizeof(OptimizedPolyObject) + (size_t)roots * sizeof(OptNode*);
	int vertices = 0, normals = 0;
	for (uint32_t i = 0; i < roots; ++i)
		if (!XwOpt_Visit(&decode, XwOpt_U32(table + i * 4), 0, &vertices, &normals))
			goto done;
	if (!XwOpt_ResolvePalettes(&decode))
		goto done;
	size_t raw_offset = XwOpt_Reserve(&decode, decode.size);
	if (decode.failed)
		goto done;
	handle = Memory_AllocHandleInternal(decode.native_size, 0, 1);
	if (!handle)
		goto done;
	OptimizedPolyObject* model = Memory_LockHandle(handle);
	uint8_t* raw_copy = (uint8_t*)model + raw_offset;
	memcpy(raw_copy, decode.bytes, decode.size);
	model->selfMarker = model;
	model->reserved = (uint16_t)(decode.bytes[4] | decode.bytes[5] << 8);
	model->rootNodeCount = (int)roots;
	model->rootNodes = (OptNode**)(model + 1);
	XwOpt_Expand(&decode, (uint8_t*)model, raw_copy);
	for (uint32_t i = 0; i < roots; ++i)
		model->rootNodes[i] = XwOpt_NodePointer(&decode, (uint8_t*)model, XwOpt_U32(table + i * 4));
	*native_size = (unsigned int)decode.native_size;
done:
	if (handle)
		XwRenderAssets_RegisterStream(XW_SOURCE_OPT, handle, file, path);
	if (XwFile_Close(file) != 0 && handle) {
		Memory_FreeHandle(handle);
		handle = 0;
	}
	free(decode.nodes);
	free(decode.bytes);
	if (!handle)
		Aeron_LogError("xw.models", "Invalid or unreadable OPT model: %s", path);
	return handle;
}

uint16_t XwOpt_Load(const char* path, int* version, unsigned int* native_size) {
	AeronFile* file = XwStorage_Open(path, "rb");
	return XwOpt_Read(file, XwStorage_LastPath(), version, native_size);
}
