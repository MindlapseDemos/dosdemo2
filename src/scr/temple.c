#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "screen.h"
#include "demo.h"
#include "3dgfx.h"
#include "gfxutil.h"
#include "util.h"
#include "mesh.h"
#include "imago2.h"
#include "3dgfx/scene.h"


static int temple_init(void);
static void destroy(void);
static void start(long trans_time);
static void stop(long trans_time);
static void draw(void);
static void shade_blobs(void);

static struct screen scr = {
	"temple",
	temple_init,
	destroy,
	start, stop,
	draw
};

static float cam_theta, cam_phi;
static float cam_dist = 8;

static struct g3d_scene *scn;
static struct g3d_mesh thing;


struct screen *temple_screen(void)
{
	return &scr;
}

static int temple_init(void)
{
	if(!(scn = scn_load("data/temple.g3d"))) {
		return -1;
	}

	if(gen_sphere_mesh(&thing, 0.5, 16, 8) == -1) {
		fprintf(stderr, "temple_init: mesh generation failed\n");
		return -1;
	}
	return 0;
}

static void destroy(void)
{
	scn_free(scn);
	destroy_mesh(&thing);
}

static void start(long trans_time)
{
	int i;

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(50, 1.333333, 0.5f, 100.0f);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_DEPTH_TEST);

	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);
	g3d_enable(G3D_LIGHT1);
	g3d_enable(G3D_LIGHT2);

	g3d_polygon_mode(G3D_GOURAUD);

	g3d_light_color(0, 1, 0.3, 0.3);
	g3d_light_color(1, 0.3, 0.3, 1);
	g3d_light_color(2, 0.3, 1, 0.3);

	for(i=0; i<3; i++) {
		g3d_light_atten(i, 0.35);
	}

	g3d_clear_color(10, 10, 10);
}

static void stop(long trans_time)
{
	g3d_disable(G3D_LIGHT0);
	g3d_disable(G3D_LIGHT1);
	g3d_disable(G3D_LIGHT2);
}

static void update(void)
{
	mouse_orbit_update(&cam_theta, &cam_phi, &cam_dist);
}

static void draw(void)
{
	int i;
	float t, tsec;
	cgm_vec3 ltpos[3];

	tsec = (float)time_msec / 1000.0f;

	update();

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();
	g3d_translate(0, -0.5, -cam_dist);
	g3d_rotate(cam_phi, 1, 0, 0);
	g3d_rotate(cam_theta, 0, 1, 0);

	for(i=0; i<3; i++) {
		t = tsec + (float)i * (CGM_PI * 2.0f / 3.0f);
		cgm_vcons(ltpos + i, cos(t) * 3.0f, 1.5, sin(t) * 3.0f);

		g3d_light_pos(i, ltpos[i].x, ltpos[i].y, ltpos[i].z);
	}

	g3d_clear(G3D_COLOR_BUFFER_BIT | G3D_DEPTH_BUFFER_BIT);

	scn_draw_meshes(scn);

	g3d_translate(0, 1, 0);
	draw_mesh(&thing);

	swap_buffers(fb_pixels);
}
