#include "xw_dos94/render/drawpol.h"
#include "xw/flight/player/user.h"
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/trace2.h"
#include <string.h>

static uint16_t line_width(uint16_t width, int32_t depth) {
	uint16_t denominator = depth < 0 ? 0 : (uint16_t)((uint32_t)depth >> 8);
	return (uint16_t)((denominator ? width / denominator : width) + 1);
}

static bool bounds_visible(const int32_t bounds[6]) {
	int32_t z = bounds[5];
	if (z < 0)
		return false;
	if (bounds[3] < 0 ? -(int64_t)z >= bounds[3] : z <= bounds[0])
		return false;
	if (bounds[4] < 0 ? -(int64_t)z >= bounds[4] : z <= bounds[1])
		return false;
	return true;
}

static void line_face(Dos94MeshProjection* p, Dos94Lighting* l, const Dos94FaceView* face,
					  Dos94RasterObject* object, uint8_t faceId) {
	const uint8_t* stream = Dos94Assets_Data(face->stream);
	uint8_t first = stream[3], second = stream[4], edge = stream[5];
	Dos94ProjectedEdge* endpoints = &p->edges[edge];
	if (!endpoints->first || !endpoints->second)
		return;
	int32_t depth =
		(int32_t)((uint32_t)(p->eyeVertices[first].z >> 1) + (uint32_t)(p->eyeVertices[second].z >> 1));
	uint16_t width = line_width(Dos94_read16(stream, 1), depth);
	Dos94ScreenPoint a = endpoints->first->screen, b = endpoints->second->screen;
	Dos94Raster* r = &Dos94_display->raster;
	object->edges[edge].face1 = faceId;
	object->edges[edge].face2 = 0;
	uint16_t flat = r->flatObjectNumber;
	Dos94_DRAWLN2_tracelineedges(&a, &b, width, l->vertexLight[first], l->vertexLight[second],
								 (uint16_t)((r->objectNumber << 8) | edge));
	r->flatObjectNumber = flat;
	object->edges[edge].halfLightIncY = (uint16_t)r->lineLightIncY >> 1;
}

/* DOS93 0x69C36E/0x69C85B: project every face before normal fallback or BSP traversal. */
static bool prepare_faces93(Dos94MeshProjection* p, Dos94Lighting* l, const Dos94Transform* transform) {
	Dos94DrawState* d = &Dos94_display->draw;
	bool visible = false;
	for (unsigned i = 0; i < p->mesh.faceCount; ++i) {
		Dos94FaceView face;
		if (!Dos94Models_Face(&p->mesh, i, &face)) {
			p->failed = true;
			return false;
		}
		int orientation;
		if (face.vertexCount == 2) {
			const uint8_t* stream = Dos94Assets_Data(face.stream);
			if (stream[5] >= p->mesh.edgeCount) {
				p->failed = true;
				return false;
			}
			Dos94ProjectedPoint* second = Dos94_TRANSFM2_calclinepts(p, l, stream[3], stream[4], &face);
			orientation = second ? 4 : 0;
			if (second) {
				p->edges[stream[5]].first = p->linePoint1;
				p->edges[stream[5]].second = second;
			}
		} else
			orientation = Dos94_TRANSFM2_getfacescreenxy(p, l, &face);
		if (p->failed)
			return false;
		d->faceVisibility[i] = (uint8_t)orientation;
		visible |= (orientation & 4) != 0 || face.twoSided;
	}
	if (!visible)
		return false;
	for (unsigned i = 0; i < p->mesh.faceCount; ++i) {
		if (d->faceVisibility[i] & 0x11)
			continue;
		Dos94FaceView face;
		if (!Dos94Models_Face(&p->mesh, i, &face)) {
			p->failed = true;
			return false;
		}
		if (Dos94_DRAWPOL_checknormal(transform, &face, p->eyeVertices, p->mesh.vertexCount))
			d->faceVisibility[i] |= 2;
	}
	return true;
}

static bool prepare_faces(Dos94MeshProjection* p, Dos94Lighting* l, const Dos94Transform* transform) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93)
		return prepare_faces93(p, l, transform);
	Dos94DrawState* d = &Dos94_display->draw;
	bool visible = false;
	for (unsigned i = 0; i < p->mesh.faceCount; ++i) {
		Dos94FaceView face;
		if (!Dos94Models_Face(&p->mesh, i, &face)) {
			p->failed = true;
			return false;
		}
		bool front = Dos94_DRAWPOL_checknormal(transform, &face, p->eyeVertices, p->mesh.vertexCount) != 0;
		d->faceVisibility[i] = front ? 1 : 16;
		if (!front && !face.twoSided)
			continue;
		if (face.vertexCount == 2) {
			const uint8_t* stream = Dos94Assets_Data(face.stream);
			Dos94ProjectedPoint* second = Dos94_TRANSFM2_calclinepts(p, l, stream[3], stream[4], &face);
			if (!second)
				continue;
			p->edges[stream[5]].first = p->linePoint1;
			p->edges[stream[5]].second = second;
		} else if (Dos94_TRANSFM2_getfacescreenxy(p, l, &face) != 4)
			continue;
		d->faceVisibility[i] |= 4;
		visible = true;
	}
	return visible;
}

static void prepare_markings(const Dos94MeshView* mesh) {
	Dos94DrawState* d = &Dos94_display->draw;
	Dos94Raster* r = &Dos94_display->raster;
	memset(d->faceMark, 0, sizeof d->faceMark);
	uint16_t count;
	if (!g_drawMarkingsFlag || !Dos94Models_ReadWord(mesh->payload, mesh->markings, &count) ||
		(int16_t)count <= 0 || count + r->numMarks >= 127)
		return;
	const uint8_t* bytes = Dos94Assets_Data(mesh->payload);
	for (unsigned i = 0; i < count; ++i) {
		unsigned offset = mesh->markings + 2 + 3 * i;
		uint16_t relative;
		if (!Dos94Models_ReadWord(mesh->payload, offset + 1, &relative))
			return;
		unsigned id = ++r->numMarks;
		d->faceMark[bytes[offset]] = id;
		d->markSources[id] = (uint16_t)(offset + relative);
	}
}

static void submit_face(Dos94MeshProjection* p, Dos94RasterObject* object, unsigned sourceFace,
						uint8_t* submitted, bool back) {
	Dos94DrawState* d = &Dos94_display->draw;
	uint8_t visibility = d->faceVisibility[sourceFace];
	if (Dos94Assets_Version() == XW_GAME_VERSION_93) {
		if (p->mesh.faceBsp && !(visibility & (back ? 0x15 : 4)))
			return;
	} else if (!(visibility & 4))
		return;
	Dos94FaceView face;
	if (!Dos94Models_Face(&p->mesh, sourceFace, &face)) {
		p->failed = true;
		return;
	}
	if (back && !face.twoSided)
		return;
	if (back)
		for (unsigned i = 0; i < 3; ++i)
			d->lighting.direction[i] = (int16_t)-d->lighting.direction[i];
	uint8_t material;
	bool ok = Dos94_DRAWPOL_getlightvalue(&d->lighting, &p->mesh, sourceFace, face.color, &material);
	if (back)
		for (unsigned i = 0; i < 3; ++i)
			d->lighting.direction[i] = (int16_t)-d->lighting.direction[i];
	if (!ok) {
		p->failed = true;
		return;
	}
	uint8_t id = *submitted;
	object->material[id] = material;
	object->gouraud[id] = d->lighting.gouraud && material < 64;
	object->modelFace[id] = sourceFace;
	if (face.vertexCount == 2)
		line_face(p, &d->lighting, &face, object, id);
	else
		Dos94_TRACE2_drawface(p, &d->lighting, &face, object, id);
	uint8_t mark = d->faceMark[sourceFace];
	if (mark)
		Dos94_DRAWPOL_drawmarkings(&p->mesh, &face, mark, d->markSources[mark], id);
	++*submitted;
}

static bool face_front(unsigned sourceFace) {
	unsigned mask = Dos94Assets_Version() == XW_GAME_VERSION_93 ? 3 : 1;
	return (Dos94_display->draw.faceVisibility[sourceFace] & mask) != 0;
}

/* DOS94 0x69AAAF / DOS93 0x69C95A: preserve both BSP branch orders. */
static void Dos94_drawpol_dobsptree(Dos94MeshProjection* p, Dos94RasterObject* object, uint8_t* submitted) {
	typedef struct Visit {
		uint16_t offset;
		uint8_t phase;
	} Visit;

	Visit stack[512];
	unsigned size = 1;
	stack[0] = (Visit) { p->mesh.faceBsp, 0 };
	const uint8_t* bytes = Dos94Assets_Data(p->mesh.payload);
	unsigned visits = 0;
	while (size && !p->failed) {
		Visit entry = stack[--size];
		uint16_t relative;
		if (++visits > 4u * p->mesh.faceCount ||
			!Dos94Models_ReadWord(p->mesh.payload, entry.offset + 1, &relative)) {
			p->failed = true;
			break;
		}
		unsigned face = bytes[entry.offset];
		if (face >= p->mesh.faceCount || size + 3 > 512) {
			p->failed = true;
			break;
		}
		bool front = face_front(face);
		if (!entry.phase) {
			stack[size++] = (Visit) { entry.offset, 1 };
			if (front && relative > 3)
				stack[size++] = (Visit) { (uint16_t)(entry.offset + 3), 0 };
			else if (!front && (int16_t)relative > 0)
				stack[size++] = (Visit) { (uint16_t)(entry.offset + relative), 0 };
		} else {
			submit_face(p, object, face, submitted, !front);
			if (front && (int16_t)relative > 0)
				stack[size++] = (Visit) { (uint16_t)(entry.offset + relative), 0 };
			else if (!front && relative > 3)
				stack[size++] = (Visit) { (uint16_t)(entry.offset + 3), 0 };
		}
	}
}

static int draw_lines(const Dos94MeshView* mesh, Dos94Transform* transform) {
	Dos94MeshProjection* p = &Dos94_display->projection;
	Dos94EyePoint eye[128];
	if (mesh->vertexCount > 128 || !Dos94_TRANSFM2_geteyecoords(transform, mesh, 0, eye) ||
		!Dos94Projection_BeginMesh(p, mesh, eye, false))
		return -1;
	Dos94Lighting* l = &Dos94_display->draw.lighting;
	Dos94Lighting_BeginMesh(l);
	if (!transform->numEyeZPositive)
		return 0;
	const uint8_t* bytes = Dos94Assets_Data(mesh->payload);
	Dos94Raster* r = &Dos94_display->raster;
	for (unsigned i = 0; i < mesh->edgeCount; ++i) {
		const uint8_t* line = bytes + mesh->streams + 5 * i;
		Dos94ProjectedPoint* second = Dos94_TRANSFM2_calclinepts(p, l, line[2], line[3], NULL);
		if (!second)
			continue;
		unsigned index = r->flatObjectNumber;
		Dos94EyePoint world = Dos94_display->bitmaps.world;
		r->flat[index] =
			(Dos94RasterFlat) { (int16_t)(world.x >> 5), (int16_t)(world.y >> 5),  (int16_t)(world.z >> 5),
								r->parentObject,         (uint8_t)r->objectNumber, line[4] };
		int32_t depth = (int32_t)((uint32_t)(eye[line[2]].z >> 1) + (uint32_t)(eye[line[3]].z >> 1));
		Dos94ScreenPoint a = p->linePoint1->screen, b = second->screen;
		Dos94_DRAWLN2_tracelineedges(&a, &b, line_width(Dos94_read16(line, 0), depth), r->lineLight1,
									 r->lineLight2, (uint16_t)(((index + 128) << 8) | (uint8_t)r->layer));
	}
	return p->failed ? -1 : 1;
}

/* DOS94 0x699C6B: original mesh, flat XY and resident line formats. */
int Dos94_drawpol_drawpolyobject(const Dos94MeshView* mesh, Dos94EyePoint origin) {
	Dos94DrawState* d = &Dos94_display->draw;
	Dos94Raster* r = &Dos94_display->raster;
	Dos94Transform* transform = (mesh->format & 1) && mesh->format != 0xFF ? &d->object : &d->camera;
	transform->origin = origin;
	Dos94Lighting* l = &d->lighting;
	l->threeD = (mesh->format & 1) && mesh->format != 0xFF;
	memcpy(l->direction, l->threeD ? d->objectLight : d->worldLight, sizeof l->direction);
	l->parent = r->parentObject;
	l->target = d->target;
	l->gateTint = d->gateTint;
	r->lastEdge = 0;
	if (mesh->format == 0xFF) {
		Dos94EyePoint eye[256];
		Dos94ScreenPoint points[512];
		Dos94ScreenBounds bounds;
		transform->numEyeZPositive = 0;
		unsigned shift = mesh->coordinateShift;
		if (shift != 8 && shift != 16)
			shift = 0;
		if (!Dos94_TRANSFM2_geteyecoordsZ0(transform, mesh, shift, eye))
			return -1;
		if (!transform->numEyeZPositive)
			return 0;
		Dos94_TRANSFM2_resetbounds(&bounds);
		uint16_t count = Dos94_TRANSFM2_getscreencoords(eye, mesh->vertexCount, points, &bounds);
		return Dos94_TRACE2_drawscreencoords(points, count, &bounds, mesh->color);
	}
	if (mesh->format == 0x40 || mesh->format == 0x41)
		return draw_lines(mesh, transform);
	uint16_t size = (r->dos93 ? 39 : 103) + 2u * mesh->faceCount + (r->dos93 ? 2u : 4u) * mesh->edgeCount;
	if (r->objectNumber >= 127 || (uint16_t)(r->nextObject + size) >= r->objectLimit)
		return -1;
	int32_t bounds[6];
	unsigned shift = r->parentObject >= 0x5000 ? 2 : 0;
	Dos94_TRANSFM2_geteyeminmax(transform, mesh->bounds, shift, bounds);
	d->closeToObject = (bounds[2] >> 2) < 0 || (uint32_t)(bounds[2] >> 2) <= d->sphereRadius;
	if (!bounds_visible(bounds))
		return 0;
	Dos94EyePoint eye[128];
	Dos94MeshProjection* p = &Dos94_display->projection;
	if (mesh->vertexCount > 128 || !Dos94_TRANSFM2_geteyecoords(transform, mesh, shift, eye) ||
		!Dos94Projection_BeginMesh(p, mesh, eye, bounds[2] < 0))
		return -1;
	Dos94Lighting_BeginMesh(l);
	if (!prepare_faces(p, l, transform))
		return p->failed ? -1 : 0;
	Dos94RasterObject* object = &r->objects[r->objectNumber];
	memset(object, 0, sizeof *object);
	object->faceCount = mesh->faceCount;
	object->parent = r->parentObject;
	object->component = Dos94_solidindex;
	Dos94EyePoint world = Dos94_display->bitmaps.world;
	object->bounds[0] = object->bounds[3] = (int16_t)(world.x >> 5);
	object->bounds[1] = object->bounds[4] = (int16_t)(world.y >> 5);
	object->bounds[2] = object->bounds[5] = (int16_t)(world.z >> 5);
	if (l->threeD)
		Dos94_TRANSFM2_getworldminmax(d->craftBasis, mesh->bounds, shift, object->bounds);
	else
		for (unsigned i = 0; i < 6; ++i)
			object->bounds[i] = (int16_t)(object->bounds[i] + (mesh->bounds[i] >> (shift ? 4 : 6)));
	r->objectById[r->objectNumber] = object;
	r->nextObject += size;
	prepare_markings(mesh);
	uint8_t faceId = 1;
	if (mesh->format & 2)
		Dos94_drawpol_dobsptree(p, object, &faceId);
	else
		for (unsigned i = 0; i < mesh->faceCount; ++i)
			submit_face(p, object, i, &faceId, !face_front(i));
	++r->objectNumber;
	if (mesh->edgeCount >= 128)
		r->objectById[r->objectNumber++] = object;
	return p->failed ? -1 : 1;
}

void Dos94_DRAWPOL_drawsurfacepoly(const Dos94EyePoint eye[4], uint16_t color) {
	Dos94ScreenPoint points[8];
	Dos94ScreenBounds bounds;
	Dos94_TRANSFM2_resetbounds(&bounds);
	uint16_t count = Dos94_TRANSFM2_getscreencoords(eye, 4, points, &bounds);
	Dos94_TRACE2_drawscreencoords(points, count, &bounds, (uint8_t)(color + 0xB0));
}
