#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "screen.h"
#include "demo.h"
#include "3dgfx.h"
#include "util.h"
#include "mesh.h"
#include "cgmath/cgmath.h"

static int samt_init(void);
static void samt_destroy(void);
static void samt_start(long trans_time);
static void samt_stop(long trans_time);
static void samt_draw(void);
static void samt_keypress(int key);

static struct screen scr = {
	"samtrans",
	samt_init, samt_destroy,
	samt_start, samt_stop,
	samt_draw,
	samt_keypress
};

static struct g3d_mesh mesh;

static const cgm_vec3 defview[] = {
	{1.847521, 1.847521, 1.847521},
	{1.847521, -1.847521, -1.847521},
	{-1.847521, 1.847521, -1.847521},
	{-1.847521, -1.847521, 1.847521}
};
static const cgm_vec3 defup[] = {
	{-0.408, 0.816, -0.408},
	{0.408, 0.816, -0.408},
	{0.408, 0.816, 0.408},
	{-0.408, 0.816, 0.408}
};

static cgm_vec3 viewpos;
static cgm_vec3 updir;
static int cur, next = -1;
static unsigned long move_start;
#define MOVE_DUR	1000

struct screen *samtrans_screen(void)
{
	return &scr;
}

static int samt_init(void)
{
	if(load_mesh(&mesh, "data/test3.obj", 0) == -1) {
		fprintf(stderr, "failed to load transition mesh\n");
		return -1;
	}

	viewpos = defview[0];
	updir = defup[0];
	return 0;
}

static void samt_destroy(void)
{
}

static void samt_start(long trans_time)
{
	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(50, 1.333333, 0.5f, 50.0f);

	g3d_disable(G3D_DEPTH_TEST);
	g3d_disable(G3D_LIGHTING);
	g3d_polygon_mode(G3D_GOURAUD);

	g3d_clear_color(0, 0, 0);
}

static void samt_stop(long trans_time)
{
}

static void update(void)
{
	float t;
	unsigned long tm;

	if(next != -1) {
		tm = time_msec - move_start;
		if(tm >= MOVE_DUR) {
			cur = next;
			next = -1;
			viewpos = defview[cur];
			updir = defup[cur];
		} else {
			t = (float)tm / (float)MOVE_DUR;
			viewpos = cgm_vvlerp(defview[cur], defview[next], t);
			updir = cgm_vvlerp(defup[cur], defup[next], t);
		}
	}
}

static void samt_draw(void)
{
	static const cgm_vec3 targ;
	float viewmat[16];

	update();

	cgm_minv_lookat(viewmat, &viewpos, &targ, &updir);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_matrix(viewmat);

	g3d_clear(G3D_COLOR_BUFFER_BIT);

	g3d_enable(G3D_ADD_BLEND);
	draw_mesh(&mesh);
	g3d_front_face(G3D_CW);
	draw_mesh(&mesh);
	g3d_front_face(G3D_CCW);
	g3d_disable(G3D_ADD_BLEND);

	swap_buffers(fb_pixels);
}

static void samt_keypress(int key)
{
	switch(key) {
	case ' ':
		move_start = time_msec;
		next = (cur + 1) & 3;
		break;

	default:
		break;
	}
}
