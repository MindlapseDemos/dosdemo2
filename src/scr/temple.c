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
#include "psys.h"
#include "imago2.h"
#include "3dgfx/scene.h"

#define SKY_ABOVE	PACK_RGB16(4, 3, 21)
#define SKY_BELOW	PACK_RGB16(19, 13, 40)

static int temple_init(void);
static void destroy(void);
static void start(long trans_time);
static void stop(long trans_time);
static void draw(void);
static void skybox(void);
static void keypress(int key);

static void distort_thing(float tm);

static struct screen scr = {
	"temple",
	temple_init,
	destroy,
	start, stop,
	draw,
	keypress
};

static float cam_theta, cam_phi = 20;
static float cam_dist = 6;

static struct g3d_scene *scn;
static struct g3d_mesh thing, sph;
static struct image star, pano;
static unsigned int hscroll_mask;

static const cgm_vec3 ltcol[3] = {
	{ 1, 0.3, 0.3 },
	{ 0.3, 0.3, 1 },
	{ 0.3, 1, 0.3 }
};
static cgm_vec3 ltpos[3];

static struct ps_emitter psys[3];

static int show_walls = 1;
static int wire;


struct screen *temple_screen(void)
{
	return &scr;
}

static int temple_init(void)
{
	int i;
	struct image tmpimg;
	uint16_t *sptr, *dptr;

	if(load_image(&star, "data/psys03.png") == -1) {
		return -1;
	}

	if(load_image(&tmpimg, "data/pano.jpg") == -1) {
		return -1;
	}
	hscroll_mask = tmpimg.width - 1;
	if(alloc_image(&pano, tmpimg.width + 320, tmpimg.height, 0) == -1) {
		return -1;
	}
	memset(pano.pixels, pano.scanlen * pano.height * 2, 0xff00);

	sptr = tmpimg.pixels;
	dptr = pano.pixels;
	for(i=0; i<pano.height; i++) {
		memcpy(dptr, sptr, tmpimg.width * 2);
		memcpy(dptr + tmpimg.width, sptr, 640);
		dptr += pano.width;
		sptr += tmpimg.width;
	}
	destroy_image(&tmpimg);

	if(!(scn = scn_load("data/temple.g3d"))) {
		return -1;
	}

	if(gen_geosphere_mesh(&sph, 1.0, 2, 0) == -1) {
		fprintf(stderr, "temple_init: mesh generation failed\n");
		return -1;
	}
	copy_mesh(&thing, &sph);

	/* create particle emitters */
	ps_init(psys);
	psys[0].pos_rad = 0.05;
	psys[0].vel_rad = 0.1;
	psys[0].spawn_rate = 20.0f;
	psys[0].life = 1.2f;
	psys[0].size0 = 0.025 * 4;
	psys[0].size1 = 0.25 * 3;
	psys[0].grav = -0.5;
	psys[0].img = &star;

	ps_init_from(psys + 1, psys);
	ps_init_from(psys + 2, psys);

	for(i=0; i<3; i++) {
		psys[i].col0.x = psys[i].col1.x = ltcol[i].x;
		psys[i].col0.y = psys[i].col1.y = ltcol[i].y;
		psys[i].col0.z = psys[i].col1.z = ltcol[i].z;
	}

	return 0;
}

static void destroy(void)
{
	scn_free(scn);
	destroy_mesh(&thing);
	destroy_image(&star);

	ps_destroy(psys);
	ps_destroy(psys + 1);
	ps_destroy(psys + 2);
}

static void start(long trans_time)
{
	int i;

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(60, 1.333333, 0.5f, 100.0f);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_DEPTH_TEST);

	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);
	g3d_enable(G3D_LIGHT1);
	g3d_enable(G3D_LIGHT2);

	g3d_polygon_mode(G3D_GOURAUD);

	for(i=0; i<3; i++) {
		g3d_light_color(i, ltcol[i].x, ltcol[i].y, ltcol[i].z);
		g3d_light_atten(i, 0.35);
	}

	g3d_clear_color(0, 0, 0);
	g3d_clear(G3D_COLOR_BUFFER_BIT);
}

static void stop(long trans_time)
{
	g3d_disable(G3D_LIGHT0);
	g3d_disable(G3D_LIGHT1);
	g3d_disable(G3D_LIGHT2);
}

static void update(float tsec)
{
	static unsigned long prev_upd;
	unsigned long interv;
	float t, dt;
	int i;

	interv = time_msec - prev_upd;
	prev_upd = time_msec;
	dt = interv / 1000.0f;

	distort_thing(tsec * 0.7);

	for(i=0; i<3; i++) {
		t = tsec + (float)i * (CGM_PI * 2.0f / 3.0f);
		cgm_vcons(ltpos + i, cos(t) * 3.0f, 1.5, sin(t) * 3.0f);

		psys[i].pos = ltpos[i];
		ps_update(psys + i, dt);
	}

	mouse_orbit_update(&cam_theta, &cam_phi, &cam_dist);
}

static void draw(void)
{
	int i;
	float tsec;

	tsec = (float)time_msec / 1000.0f;

	update(tsec);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();
	g3d_translate(0, -1, -cam_dist);
	g3d_rotate(cam_phi, 1, 0, 0);
	g3d_rotate(cam_theta, 0, 1, 0);

	for(i=0; i<3; i++) {
		g3d_light_pos(i, ltpos[i].x, ltpos[i].y, ltpos[i].z);
	}

	g3d_clear(G3D_DEPTH_BUFFER_BIT);

	if(show_walls) {
		scn_draw_meshes(scn);
	} else {
		skybox();
	}

	g3d_mtl_diffuse(1, 1, 1);
	g3d_mtl_specular(1, 1, 1);
	g3d_mtl_shininess(20.0);

	g3d_enable(G3D_SPECULAR);

	g3d_push_matrix();
	g3d_translate(0, 1.5, 0);
	g3d_rotate(tsec * 30.0f, 1, 0, 0);
	g3d_rotate(tsec * 40.0f, 0, 1, 0);

	if(wire) {
		g3d_polygon_mode(G3D_WIRE);
		draw_mesh(&thing);
		g3d_polygon_mode(G3D_GOURAUD);
	} else {
		draw_mesh(&thing);
	}
	g3d_pop_matrix();
	g3d_disable(G3D_SPECULAR);

	for(i=0; i<3; i++) {
		ps_draw(psys + i);
	}

	g3d_polygon_mode(G3D_FLAT);
	g3d_disable(G3D_LIGHTING);
	g3d_enable(G3D_TEXTURE_2D | G3D_ADD_BLEND);
	g3d_set_texture(star.width, star.height, star.pixels);
	for(i=0; i<3; i++) {
		unsigned int r = (unsigned int)(ltcol[i].x * 255.0f);
		unsigned int g = (unsigned int)(ltcol[i].y * 255.0f);
		unsigned int b = (unsigned int)(ltcol[i].z * 255.0f);
		draw_billboard(ltpos[i].x, ltpos[i].y, ltpos[i].z, 0.8, r, g, b, 255);
	}
	g3d_disable(G3D_TEXTURE_2D | G3D_ADD_BLEND);
	g3d_enable(G3D_LIGHTING);
	g3d_polygon_mode(G3D_GOURAUD);

	swap_buffers(fb_pixels);
}

static void distort_thing(float tm)
{
	int i;
	float s, v_i, v_j, v_k;
	cgm_vec3 v;
	struct g3d_vertex *vsrc = sph.varr;
	struct g3d_vertex *vdest = thing.varr;

	for(i=0; i<sph.vcount; i++) {
		cgm_vcons(&v, vsrc->x, vsrc->y, vsrc->z);

		v_i = cgm_vvdot(v, cgm_vvec(1, 0, 0));
		v_j = cgm_vvdot(v, cgm_vvec(0, 1, 0));
		v_k = cgm_vvdot(v, cgm_vvec(0, 0, 1));

		s = sin(v_i * cos(tm) * 5.0f) + cos(v_i * cos(tm * 2.0f) * 3.0f);
		s += sin(v_j * 5.0f * sin(tm)) * 0.8 + cos(v_k * cos(tm * 3.0f) * 2.8f);
		s = s * 0.15f + 0.85f;

		vdest->x = v.x * s;
		vdest->y = v.y * s;
		vdest->z = v.z * s;

		vsrc++;
		vdest++;
	}

	calc_mesh_normals(&thing);
}

static void skybox(void)
{
	float right_x, right_z, up_y, up_z, d;
	const float *projmat;
	const float *viewmat;
	int i, top, bot, hscroll, hor_row, nrows;
	uint16_t *fbptr, *sptr;

	viewmat = g3d_get_matrix(G3D_MODELVIEW, 0);
	up_y = viewmat[5];
	up_z = -viewmat[6];
	right_x = viewmat[0];
	right_z = -viewmat[2];

	projmat = g3d_get_matrix(G3D_PROJECTION, 0);
	d = projmat[5];

	hor_row = 120 + cround64(d * 120.0f * up_z / up_y - 0.5);
	hscroll = cround64((atan2(right_z, right_x) + CGM_PI) * 160.0f) & hscroll_mask;

	top = hor_row - pano.height / 2;
	bot = top + pano.height;

	if(bot <= 0) {
		memset16(fb_pixels, SKY_BELOW, 320 * 240);
		return;
	}
	if(top >= 240) {
		memset16(fb_pixels, SKY_ABOVE, 320 * 240);
		return;
	}

	if(top < 0) {
		fbptr = fb_pixels;
		sptr = pano.pixels + pano.scanlen * -top;
		top = 0;
	} else {
		fbptr = fb_pixels + 320 * top;
		sptr = pano.pixels;
	}
	if(bot > 240) bot = 240;

	if(top > 0) {
		memset16(fb_pixels, SKY_ABOVE, top * 320);
	}
	if(bot < 240) {
		memset16(fb_pixels + bot * 320, SKY_BELOW, (240 - bot) * 320);
	}

	sptr += hscroll;

	i = top;
	while(i++ < bot) {
		memcpy(fbptr, sptr, 640);
		fbptr += 320;
		sptr += pano.scanlen;
	}
}

static void keypress(int key)
{
	switch(key) {
	case 'w':
		wire ^= 1;
		break;

	case ' ':
		show_walls ^= 1;
		break;

	default:
		break;
	}
}
