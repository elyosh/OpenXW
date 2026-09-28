#include "xw_dos94/render/dos93_order.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/draw.h"
#include "xw_dos94/render/fview.h"
#include <string.h>

typedef struct CameraSide {
	int16_t side, forward, up;
	unsigned shift;
} CameraSide;

typedef struct ComponentOrder {
	uint16_t* items;
	unsigned count;
} ComponentOrder;

/* Fighters use one normalization; threshold-based orders double the high-word
 * mask before taking its magnitude, then halve the normalized delta once more. */
static CameraSide relative_eye(const ObjectRecord* object, bool fighter) {
	int32_t delta[3] = {
		(int32_t)(2u * ((uint32_t)g_flightCamera.worldPosition.x - (uint32_t)object->worldX)),
		(int32_t)(2u * ((uint32_t)g_flightCamera.worldPosition.y - (uint32_t)object->worldY)),
		(int32_t)(2u * ((uint32_t)g_flightCamera.worldPosition.z - (uint32_t)object->worldZ))
	};
	uint16_t magnitude = 0;
	for (unsigned i = 0; i < 3; ++i) {
		int16_t high = (int16_t)(delta[i] >> 16);
		if (!fighter)
			high = (int16_t)((uint16_t)high * 2u);
		magnitude |= high < 0 ? (uint16_t)-high : (uint16_t)high;
	}
	unsigned shift = 0;
	do {
		++shift;
		magnitude >>= 1;
		for (unsigned i = 0; i < 3; ++i)
			delta[i] >>= 1;
	} while (magnitude);
	int16_t point[3];
	for (unsigned i = 0; i < 3; ++i)
		point[i] = (int16_t)(fighter ? delta[i] : delta[i] >> 1);
	const int16_t side[3] = { object->cachedSideX, object->cachedSideY, object->cachedSideZ };
	const int16_t forward[3] = { object->cachedForwardX, object->cachedForwardY, object->cachedForwardZ };
	const int16_t up[3] = { object->cachedUpX, object->cachedUpY, object->cachedUpZ };
	return (CameraSide) { Dos94_math2_dot3Q15(side, point), Dos94_math2_dot3Q15(forward, point),
						  Dos94_math2_dot3Q15(up, point), shift };
}

static void append(ComponentOrder* order, uint16_t component) {
	if (order->count < DOS_COMPONENT_ORDER_CAPACITY)
		order->items[order->count++] = component;
}

static void triple(ComponentOrder* order, uint16_t a, uint16_t b, uint16_t c) {
	append(order, a);
	append(order, b);
	append(order, c);
}

/* DOS93 DRAW_drawxwing / DRAW_drawshuttle: strict signed thresholds. */
static void xwing_order(const CameraSide* eye, uint16_t* out) {
	static const uint16_t orders[6][5] = { { 4, 3, 0, 2, 1 }, { 2, 1, 0, 4, 3 }, { 0, 2, 4, 1, 3 },
										   { 3, 4, 0, 1, 2 }, { 1, 2, 0, 3, 4 }, { 0, 1, 3, 2, 4 } };
	unsigned row = (eye->up < 0 ? 0 : 3) + (eye->side > 26 ? 0 : eye->side < -26 ? 1 : 2);
	memcpy(out, orders[row], sizeof orders[row]);
}

static void shuttle_order(const CameraSide* eye, uint16_t* out) {
	uint16_t a = eye->side < 0 ? 1 : 2, b = eye->side < 0 ? 2 : 1;
	ComponentOrder order = { out, 0 };
	if (eye->up > 107) {
		append(&order, 3);
		triple(&order, 0, a, b);
	} else if (eye->up < -90) {
		triple(&order, a, b, 0);
		append(&order, 3);
	} else {
		triple(&order, 0, a, b);
		append(&order, 3);
	}
}

static void bwing_order(CameraSide eye, const CraftData* craft, uint16_t* out) {
	uint16_t angle = (uint16_t)craft->meshRotation[1] << 8;
	if (angle == 0x4000) {
		int16_t side = eye.side;
		eye.side = (int16_t)-eye.up;
		eye.up = side;
	} else if (angle) {
		int16_t sine = Dos94_trig2_getsignedsin(angle), cosine = Dos94_trig2_getsignedcos(angle);
		int16_t side = Dos94_math2_dot2Q15(cosine, (int16_t)-sine, eye.side, eye.up);
		eye.up = Dos94_math2_dot2Q15(sine, cosine, eye.side, eye.up);
		eye.side = side;
	}
	ComponentOrder wings = { out + (eye.up < -49 ? 0 : 3), 0 };
	if (eye.side < -43)
		triple(&wings, 5, 3, 4);
	else if (eye.side > 43)
		triple(&wings, 4, 3, 5);
	else
		triple(&wings, 3, eye.side < 0 ? 4 : 5, eye.side < 0 ? 5 : 4);
	ComponentOrder body = { out + (eye.up < -49 ? 3 : 0), 0 };
	if (eye.up < 49)
		triple(&body, 2, 1, 0);
	else if (eye.up < 136)
		triple(&body, 1, 0, 2);
	else
		triple(&body, 0, 1, 2);
}

static void three_component_order(const CameraSide* eye, uint16_t model, uint16_t* out) {
	static const int16_t thresholds[18] = { [2] = 100, [4] = 38,   [5] = 65, [6] = 105,
											[7] = 48,  [12] = 775, [17] = 60 };
	int16_t threshold = (int16_t)(2 * thresholds[model]) >> (eye->shift + 1);
	ComponentOrder order = { out, 0 };
	if (eye->side > threshold)
		triple(&order, 2, 0, 1);
	else if (eye->side < (int16_t)-threshold)
		triple(&order, 1, 0, 2);
	else
		triple(&order, 0, 1, 2);
}

static void corvette_order(const CameraSide* eye, uint16_t* out) {
	int16_t forward = (int16_t)-eye->forward;
	int16_t fore = -990 >> eye->shift, mid = -183 >> eye->shift;
	int16_t rear = 1431 >> eye->shift, engine = 2896 >> eye->shift;
	int16_t height = 224 >> eye->shift;
	ComponentOrder order = { out, 0 };
	if (forward < mid) {
		append(&order, forward < fore ? 0 : 1);
		append(&order, forward < fore ? 1 : 0);
	} else if (forward >= engine) {
		append(&order, 4);
		append(&order, 3);
	} else if (forward >= rear)
		append(&order, 3);
	if (eye->up > height)
		triple(&order, 5, 2, 6);
	else if (eye->up > (int16_t)-height)
		triple(&order, 2, 5, 6);
	else
		triple(&order, 6, 2, 5);
	if (forward >= mid) {
		append(&order, 1);
		append(&order, 0);
	}
	if (forward < rear)
		append(&order, 3);
	if (forward < engine)
		append(&order, 4);
}

static void frigate_order(const CameraSide* eye, uint16_t* out) {
	bool below = eye->up < (-837 >> eye->shift);
	out[0] = out[2] = below ? 3 : 2;
	out[1] = out[3] = below ? 2 : 3;
	int16_t forward = (int16_t)-eye->forward;
	if (forward > (3024 >> eye->shift)) {
		out[0] = 0;
		out[1] = 1;
	} else {
		unsigned pair = forward > (-3522 >> eye->shift) ? 0 : 2;
		out[pair] = 1;
		out[pair + 1] = 0;
	}
}

static void destroyer_generators(ComponentOrder* order, int16_t side) {
	append(order, side < 0 ? 1 : 2);
	append(order, side < 0 ? 2 : 1);
}

static void destroyer_lower_group(ComponentOrder* order, int16_t side) {
	triple(order, side < 0 ? 6 : 8, 7, side < 0 ? 8 : 6);
}

static void destroyer_upper_group(ComponentOrder* order, int16_t side, int16_t left, int16_t right) {
	if (left < 0 && right < 0)
		triple(order, side > 0 ? 11 : 12, side > 0 ? 12 : 11, 10);
	else if (left < 0)
		triple(order, 12, 10, 11);
	else if (right < 0)
		triple(order, 11, 10, 12);
	else
		triple(order, 10, side > 0 ? 11 : 12, side > 0 ? 12 : 11);
}

/* DOS93 starship_DrawModel16ComponentOrder: authored oblique planes determine
 * group placement. Keep repeated entries and the original first-13 consumption;
 * a short generated prefix leaves the previous order's tail intact. */
static void destroyer_order(const CameraSide* eye, uint16_t* out) {
	int16_t rear = 0x37E8 >> (eye->shift + 1), top = 0x58C8 >> (eye->shift + 1);
	int16_t base = 0x1500 >> (eye->shift + 1), sideOrigin = 0x4830 >> eye->shift;
	int16_t forwardOrigin = 0x53C4 >> eye->shift;
	int16_t upperOrigin = (int16_t)0xFBE0 >> eye->shift, lowerOrigin = (int16_t)0xFE80 >> eye->shift;
	int16_t leftPoint[3] = { (int16_t)(eye->side - sideOrigin), (int16_t)-(eye->forward + forwardOrigin),
							 (int16_t)(eye->up - upperOrigin) };
	int16_t rightPoint[3] = { (int16_t)(eye->side + sideOrigin), leftPoint[1], leftPoint[2] };
	int16_t lowerPoint[3] = { eye->side, leftPoint[1], (int16_t)(eye->up - lowerOrigin) };
	static const int16_t normals[6][3] = {
		{ (int16_t)0x84FB, 0x2357, 0 }, { (int16_t)0xE116, 0x08DE, 0x7BE3 },
		{ 0x7B05, 0x2357, 0 },          { 0x1EEA, 0x08DE, 0x7BE3 },
		{ 0, (int16_t)0x91DA, 0x4130 }, { 0, (int16_t)0x9630, (int16_t)0xB7FD }
	};
	int16_t leftSide = Dos94_math2_dot3Q15(normals[0], leftPoint);
	int16_t leftUpper = Dos94_math2_dot3Q15(normals[1], leftPoint);
	int16_t rightSide = Dos94_math2_dot3Q15(normals[2], rightPoint);
	int16_t rightUpper = Dos94_math2_dot3Q15(normals[3], rightPoint);
	int16_t forwardLower = Dos94_math2_dot3Q15(normals[4], lowerPoint);
	int16_t rearLower = Dos94_math2_dot3Q15(normals[5], lowerPoint);
	ComponentOrder order = { out, 0 };
	if (eye->forward >= rear)
		append(&order, 9);
	if (leftSide < 0)
		append(&order, 5);
	if (rightSide < 0)
		append(&order, 4);
	if (eye->up > lowerOrigin) {
		if (eye->up > base) {
			if (eye->up < top)
				append(&order, 0);
			destroyer_generators(&order, eye->side);
			if (eye->up >= top)
				append(&order, 0);
		}
		if (forwardLower < 0)
			destroyer_lower_group(&order, eye->side);
		append(&order, 3);
		if (eye->up <= base) {
			append(&order, 0);
			destroyer_generators(&order, eye->side);
		}
		if (rearLower < 0 && forwardLower >= 0)
			destroyer_lower_group(&order, eye->side);
		destroyer_upper_group(&order, eye->side, leftUpper, rightUpper);
		if (rearLower >= 0 && forwardLower >= 0)
			destroyer_lower_group(&order, eye->side);
	} else {
		if (rearLower < 0)
			destroyer_lower_group(&order, eye->side);
		destroyer_upper_group(&order, eye->side, leftUpper, rightUpper);
		if (rearLower >= 0 && forwardLower < 0)
			destroyer_lower_group(&order, eye->side);
		append(&order, 3);
		/* The original repeats this predicate after component 3. */
		if (rearLower >= 0 && forwardLower < 0)
			destroyer_lower_group(&order, eye->side);
		append(&order, 0);
		destroyer_generators(&order, eye->side);
	}
	if (leftSide >= 0)
		append(&order, 5);
	if (rightSide >= 0)
		append(&order, 4);
	if (eye->forward < rear)
		append(&order, 9);
}

void Dos93_DRAW_drawcomplexobject(uint16_t index) {
	ObjectRecord* object = &g_objectTable[index];
	uint16_t model = object->objectType == 119 ? 16 : object->objectType;
	Dos94_DRAWPOL_setmarkingcolors(object->markings);
	uint16_t savedDetail = g_shipDetailPolyCount;
	bool distant = (model == 13 || model == 16) &&
				   Dos94_display->bitmaps.eye.z > (int32_t)((uint32_t)g_starshipDetail << 15);
	if (distant || model == 3 || model == 8 || model == 10 || model == 11) {
		Dos94_display->raster.parentObject = index + (distant ? 0x7000 : 0);
		if (distant)
			g_shipDetailPolyCount = 16;
		Dos94_DRAW_drawcomponent(model, distant ? (model == 13 ? 2 : 13) : 0, Dos94_display->bitmaps.eye);
	} else {
		uint16_t* order = Dos94_display->draw.componentOrder;
		CameraSide eye = relative_eye(object, model == 1 || model == 9 || model == 118);
		unsigned count = 0;
		switch (model) {
			case 1:
				xwing_order(&eye, order);
				count = 5;
				break;
			case 9:
				shuttle_order(&eye, order);
				count = 4;
				break;
			case 118:
				if (object->instanceData) {
					bwing_order(eye, object->instanceData, order);
					count = 6;
				}
				break;
			case 2:
			case 4:
			case 5:
			case 6:
			case 7:
			case 12:
			case 17:
				three_component_order(&eye, model, order);
				count = 3;
				break;
			case 13:
				order[0] = (int16_t)-eye.forward < (2580 >> eye.shift) ? 0 : 1;
				order[1] = 1 - order[0];
				count = 2;
				break;
			case 14:
				frigate_order(&eye, order);
				count = 4;
				g_shipDetailPolyCount = 16;
				break;
			case 15:
				corvette_order(&eye, order);
				count = 7;
				break;
			case 16:
				destroyer_order(&eye, order);
				count = 13;
				break;
		}
		if (count)
			Dos94_DRAW_drawcraft(index, model, order, count);
	}
	g_shipDetailPolyCount = savedDetail;
	Dos94_DRAWPOL_setmarkingcolors(0);
}
