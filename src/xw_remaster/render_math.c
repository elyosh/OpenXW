/* Matrix conventions and layout helpers adapted from OpenXvT f643323. */
#include "xw_remaster/render_math.h"
#include "aeron/asset/opt_model.h"
#include <math.h>
#include <string.h>

static float Dot(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

static int Normalize(float v[3]) {
	float length = sqrtf(Dot(v, v));
	if (!isfinite(length) || length < 1e-6f)
		return 0;
	for (int i = 0; i < 3; ++i)
		v[i] /= length;
	return 1;
}

static void fl_rows_to_quat(const float m[9], float q[4]) {
	const float trace = m[0] + m[4] + m[8];
	if (trace > 0.0f) {
		const float sq = sqrtf(trace + 1.0f) * 2.0f;
		q[0] = 0.25f * sq;
		q[1] = (m[7] - m[5]) / sq;
		q[2] = (m[2] - m[6]) / sq;
		q[3] = (m[3] - m[1]) / sq;
	} else if (m[0] > m[4] && m[0] > m[8]) {
		const float sq = sqrtf(1.0f + m[0] - m[4] - m[8]) * 2.0f;
		q[0] = (m[7] - m[5]) / sq;
		q[1] = 0.25f * sq;
		q[2] = (m[1] + m[3]) / sq;
		q[3] = (m[2] + m[6]) / sq;
	} else if (m[4] > m[8]) {
		const float sq = sqrtf(1.0f + m[4] - m[0] - m[8]) * 2.0f;
		q[0] = (m[2] - m[6]) / sq;
		q[1] = (m[1] + m[3]) / sq;
		q[2] = 0.25f * sq;
		q[3] = (m[5] + m[7]) / sq;
	} else {
		const float sq = sqrtf(1.0f + m[8] - m[0] - m[4]) * 2.0f;
		q[0] = (m[3] - m[1]) / sq;
		q[1] = (m[2] + m[6]) / sq;
		q[2] = (m[5] + m[7]) / sq;
		q[3] = 0.25f * sq;
	}
}

static void fl_curmat_from_cached(const int16_t rows[9], float cur[3][3]) {
	const float q = 1.0f / 32768.0f;
	cur[0][0] = rows[0] * q;
	cur[0][1] = rows[1] * q;
	cur[0][2] = rows[2] * q;
	cur[1][0] = rows[6] * q;
	cur[1][1] = rows[7] * q;
	cur[1][2] = rows[8] * q;
	cur[2][0] = -rows[3] * q;
	cur[2][1] = -rows[4] * q;
	cur[2][2] = -rows[5] * q;
}

#define FL_Q16_TO_RAD (2.0f * 3.14159265358979323846f / 65536.0f)

/* Rodrigues rotation of all three curMat rows about `axis` by a Q16
 * angle — the float mirror of FVIEW_transformaxes (new = M.row with
 * new_x = m00 x + m10 y + m20 z, i.e. M applied transposed). */
static void fl_transformaxes(float cur[3][3], const float axis[3], int angle_q16) {
	if ((int16_t)angle_q16 == 0) {
		return;
	}
	const float a = (float)(int16_t)angle_q16 * FL_Q16_TO_RAD;
	const float c = cosf(a);
	const float sn = sinf(a);
	const float t = 1.0f - c;
	const float x = axis[0], y = axis[1], z = axis[2];
	/* m[i][j] laid out as FVIEW_transformaxes computes m00..m22. */
	const float m[3][3] = {
		{ c + t * x * x, sn * z + t * y * x, -sn * y + t * z * x },
		{ -sn * z + t * y * x, c + t * y * y, sn * x + t * z * y },
		{ sn * y + t * z * x, -sn * x + t * z * y, c + t * z * z },
	};
	for (int r = 0; r < 3; r++) {
		const float ox = cur[r][0], oy = cur[r][1], oz = cur[r][2];
		/* new_x = m00 ox + m10 oy + m20 oz (the engine's application). */
		cur[r][0] = m[0][0] * ox + m[1][0] * oy + m[2][0] * oz;
		cur[r][1] = m[0][1] * ox + m[1][1] * oy + m[2][1] * oz;
		cur[r][2] = m[0][2] * ox + m[1][2] * oy + m[2][2] * oz;
	}
}

/* curMat rows from the record's Q16 Euler angles — the float mirror of
 * FVIEW_calcrotatemove + FVIEW_calcrotateorient (statics and dirty
 * orientations). */
static void fl_curmat_from_euler(const XwSnapObject* f, float cur[3][3]) {
	const float aA = (float)(int16_t)(0xc000 - f->pitch) * FL_Q16_TO_RAD;
	const float aB = (float)(int16_t)(-(int16_t)f->yaw) * FL_Q16_TO_RAD;
	const float cB = cosf(aB), sB = sinf(aB);
	const float cA = cosf(aA), sA = sinf(aA);
	cur[0][0] = cB;
	cur[0][1] = sB;
	cur[0][2] = 0.0f;
	cur[2][0] = -sB * cA;
	cur[2][1] = cB * cA;
	cur[2][2] = sA;
	cur[1][0] = -sB * sA;
	cur[1][1] = cB * sA;
	cur[1][2] = -cA;
	fl_transformaxes(cur, cur[2], (int16_t)f->roll);
}

/* Model->world basis rows: the curMat rows in the engine's consumption
 * order (R0, R2, R1) — FVIEW_ComputeObjectViewMatrix's row read, with
 * the camera factor moved into the scene's view matrix. */
static void fl_object_world(const float cur[3][3], float out[9]) {
	const float* src[3] = { cur[0], cur[2], cur[1] };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			out[i * 3 + j] = src[i][j];
		}
	}
}

/* Row-major affine matrix: model basis vectors become columns. */
static void fl_model_matrix(const float basis[9], const float delta[3], float scale, float m[16]) {
	m[0] = scale * basis[0];
	m[1] = scale * basis[3];
	m[2] = scale * basis[6];
	m[3] = delta[0];
	m[4] = scale * basis[1];
	m[5] = scale * basis[4];
	m[6] = scale * basis[7];
	m[7] = delta[1];
	m[8] = scale * basis[2];
	m[9] = scale * basis[5];
	m[10] = scale * basis[8];
	m[11] = delta[2];
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

bool XwRenderMath_Layout(float sw, float sh, float tw, float th, XwLayoutTransform* out) {
	if (!out || !isfinite(sw + sh + tw + th) || sw <= 0 || sh <= 0 || tw <= 0 || th <= 0)
		return false;
	float scale = fminf(tw / 640.0f, th / 480.0f);
	float width = scale * 640, height = scale * 480;
	/* Match Aeron's presentation fit: tiny aspect shortfalls must not leave HUD hairlines. */
	if ((tw - width) * 256 <= tw)
		width = tw;
	if ((th - height) * 256 <= th)
		height = th;
	*out = (XwLayoutTransform) { width / sw, height / sh, sw, sh, tw, th };
	return true;
}

void XwRenderMath_LayoutPoint(const XwLayoutTransform* t, float ax, float ay, float x, float y, float* ox,
							  float* oy) {
	*ox = x * t->scale_x + ax * (t->target_width - t->source_width * t->scale_x);
	*oy = y * t->scale_y + ay * (t->target_height - t->source_height * t->scale_y);
}

void XwRenderMath_LayoutInverse(const XwLayoutTransform* t, float ax, float ay, float x, float y, float* ox,
								float* oy) {
	*ox = (x - ax * (t->target_width - t->source_width * t->scale_x)) / t->scale_x;
	*oy = (y - ay * (t->target_height - t->source_height * t->scale_y)) / t->scale_y;
}

AeronRectI XwRenderMath_LayoutRect(const XwLayoutTransform* t) {
	int w = (int)lroundf(t->source_width * t->scale_x), h = (int)lroundf(t->source_height * t->scale_y);
	return (AeronRectI) { ((int)t->target_width - w) / 2, ((int)t->target_height - h) / 2, w, h };
}

void XwRenderMath_Local(const int32_t origin[3], const int32_t world[3], float out[3]) {
	for (unsigned i = 0; i < 3; ++i)
		out[i] = (float)(int32_t)((uint32_t)world[i] - (uint32_t)origin[i]);
}

bool XwRenderMath_SameObject(XwSnapObjectId a, XwSnapObjectId b) {
	return a.kind == b.kind &&
		   (a.kind == XW_SNAP_OBJECT_NONE || (a.slot == b.slot && a.generation == b.generation));
}

static bool CameraBasis(const float source[9], float quat[4]) {
	float rows[9];
	memcpy(rows, source, sizeof rows);
	if (!Normalize(rows))
		return false;
	float projection = Dot(rows, rows + 3);
	for (unsigned i = 0; i < 3; ++i)
		rows[i + 3] -= rows[i] * projection;
	if (!Normalize(rows + 3))
		return false;
	float forward[3] = { rows[1] * rows[5] - rows[2] * rows[4], rows[2] * rows[3] - rows[0] * rows[5],
						 rows[0] * rows[4] - rows[1] * rows[3] };
	/* The source rows and Aeron both use right, eye-down, forward. */
	if (Dot(forward, rows + 6) <= 0)
		return false;
	memcpy(rows + 6, forward, sizeof forward);
	fl_rows_to_quat(rows, quat);
	float norm = sqrtf(quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]);
	if (!isfinite(norm) || norm < 1e-6f)
		return false;
	for (unsigned i = 0; i < 4; ++i)
		quat[i] /= norm;
	return true;
}

bool XwRenderMath_BuildMainView(const XwSnapCamera* cam, const int32_t origin[3], int width, int height,
								XwRenderView* out) {
	if (!cam || !origin || !out || cam->viewport.width <= 0 || cam->viewport.height <= 0 || cam->focal_x <= 0)
		return false;
	XwLayoutTransform layout;
	if (!XwRenderMath_Layout(cam->screen_width, cam->screen_height, width, height, &layout))
		return false;
	memset(out, 0, sizeof *out);
	AeronSceneCamera* c = &out->camera;
	if (!CameraBasis(cam->rows, c->ori))
		return false;
	memcpy(out->origin_world, origin, sizeof out->origin_world);
	XwRenderMath_Local(origin, cam->world_pos, c->pos);
	float aspect =
		!cam->aspect_y_q16 || cam->aspect_y_q16 == UINT16_MAX ? 1.0f : cam->aspect_y_q16 / 65536.0f;
	float fx = cam->focal_x * layout.scale_x, fy = cam->focal_x * aspect * layout.scale_y;
	c->h_half_rad = atanf(width / (2 * fx));
	c->v_half_rad = atanf(height / (2 * fy));
	c->near_z = 1;
	float cx, cy;
	XwRenderMath_LayoutPoint(&layout, .5f, .5f, cam->viewport.x + cam->center_x,
							 cam->viewport.y + cam->center_y + cam->projection_offset_y, &cx, &cy);
	c->proj_x_offset = 2 * cx / width - 1;
	c->proj_y_offset = 1 - 2 * cy / height;
	c->viewport = (AeronRectI) { 0, 0, width, height };
	out->classic_pixel_scale_x = layout.scale_x;
	out->classic_pixel_scale_y = layout.scale_y;
	AeronScene_ComputeViewProj(c, out->view_proj);
	return true;
}

bool XwRenderMath_ProjectWorld(const XwRenderView* view, const int32_t world[3], float* x, float* y,
							   float* depth) {
	if (!view || !world || !x || !y)
		return false;
	float p[3];
	XwRenderMath_Local(view->origin_world, world, p);
	const float* m = view->view_proj;
	float w = m[12] * p[0] + m[13] * p[1] + m[14] * p[2] + m[15];
	if (depth)
		*depth = w;
	if (w <= 0 || !isfinite(w))
		return false;
	float nx = (m[0] * p[0] + m[1] * p[1] + m[2] * p[2] + m[3]) / w;
	float ny = (m[4] * p[0] + m[5] * p[1] + m[6] * p[2] + m[7]) / w;
	*x = (nx + 1) * .5f * view->camera.viewport.width + view->camera.viewport.x;
	*y = (1 - ny) * .5f * view->camera.viewport.height + view->camera.viewport.y;
	return isfinite(*x) && isfinite(*y);
}

void XwRenderMath_ObjectMatrix(const XwSnapObject* object, uint8_t version, const int32_t origin[3],
							   float out[16]) {
	float cur[3][3], basis[9], local[3];
	if (object->id.kind == XW_SNAP_OBJECT_MOBILE && !object->orientation_dirty)
		fl_curmat_from_cached(object->cached_rows_q15, cur);
	else
		fl_curmat_from_euler(object, cur);
	fl_object_world(cur, basis);
	XwRenderMath_Local(origin, object->world_pos, local);
	fl_model_matrix(basis, local, version == 98 ? AERON_OPT_UNITS_PER_METER : 1.0f, out);
}

void XwRenderMath_DosMeshMatrix(const float pose[16], uint8_t format, uint8_t shift, bool enlarged,
								float out[16]) {
	if (out != pose)
		memcpy(out, pose, 16 * sizeof(float));
	/* Even formats are world-aligned; flat geometry uses the main camera too. */
	if (!(format & 1) || format == 0xFF)
		for (unsigned row = 0; row < 3; ++row)
			for (unsigned col = 0; col < 3; ++col)
				out[row * 4 + col] = row == col ? 1 : 0;
	float scale = format == 0xFF ? ldexpf(1, shift == 8 || shift == 16 ? shift : 0)
								 : (format != 0x40 && format != 0x41 && enlarged ? 2.0f : .5f);
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned col = 0; col < 3; ++col)
			out[row * 4 + col] *= scale;
}

void XwRenderMath_ViewRows(const AeronSceneCamera* camera, float m[9]) {
	const float* q = camera->ori;
	const float w = q[0], x = q[1], y = q[2], z = q[3];
	const float xx = x * x, yy = y * y, zz = z * z;
	const float xy = x * y, xz = x * z, yz = y * z;
	const float wx = w * x, wy = w * y, wz = w * z;
	m[0] = 1.0f - 2.0f * (yy + zz);
	m[1] = 2.0f * (xy - wz);
	m[2] = 2.0f * (xz + wy);
	m[3] = 2.0f * (xy + wz);
	m[4] = 1.0f - 2.0f * (xx + zz);
	m[5] = 2.0f * (yz - wx);
	m[6] = 2.0f * (xz - wy);
	m[7] = 2.0f * (yz + wx);
	m[8] = 1.0f - 2.0f * (xx + yy);
}
