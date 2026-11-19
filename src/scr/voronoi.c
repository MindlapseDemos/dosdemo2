#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "screen.h"
#include "demo.h"
#include "3dgfx.h"
#include "util.h"
#include "mesh.h"
#include "cgmath/cgmath.h"

static int voronoi_init(void);
static void voronoi_destroy(void);
static void voronoi_start(long trans_time);
static void voronoi_stop(long trans_time);
static void voronoi_draw(void);
static void voronoi_keypress(int key);

static struct screen scr = {
	"voronoi",
	voronoi_init, voronoi_destroy,
	voronoi_start, voronoi_stop,
	voronoi_draw,
	voronoi_keypress
};

typedef struct Point2D
{
	int x, y;
} Point2D;

#define VORONOI_TEX_W 128
#define VORONOI_TEX_H 128

#define VORONOI_POINTS 16

static Point2D points[VORONOI_POINTS];
static unsigned char texture[VORONOI_TEX_W * VORONOI_TEX_H];


struct screen * voronoi_screen(void)
{
	return &scr;
}

static int voronoi_init(void)
{
	int i;

	for (i = 0; i < VORONOI_POINTS; ++i) {
		points[i].x = rand() & (VORONOI_TEX_W - 1);
		points[i].y = rand() & (VORONOI_TEX_H - 1);
	}

	return 0;
}

static void voronoi_destroy(void)
{
}

static void voronoi_start(long trans_time)
{
}

static void voronoi_stop(long trans_time)
{
}

static void updateVoronoiPoints()
{
	int i;

	float t1 = time_msec * 0.0003f;
	float t2 = time_msec * 0.0007f;
	float t3 = time_msec * 0.0004f;
	float t4 = time_msec * 0.0007f;

	float dx1 = VORONOI_TEX_W * 0.375f;
	float dx2 = VORONOI_TEX_W * 0.45f;
	float dy1 = VORONOI_TEX_H * 0.325f;
	float dy2 = VORONOI_TEX_H * 0.25f;

	for (i = 0; i < VORONOI_POINTS; ++i) {
		float it1 = (float)i * 1.77f;
		float it2 = (float)i * 2.36f;

		int x = VORONOI_TEX_W / 2 + (int)(sin(t1 + it1) * dx1 + sin(t2 + it2) * dx2);
		int y = VORONOI_TEX_H / 2 + (int)(sin(t3 - it2) * dy1 + sin(t3 + it1) * dy2);

		points[i].x = x & (VORONOI_TEX_W - 1);
		points[i].y = y & (VORONOI_TEX_H - 1);
	}
}

static void updateVoronoiTexture()
{
	int x, y, i;
	unsigned int c;

	unsigned char *dst = texture;
	for (y = 0; y < VORONOI_TEX_H; ++y) {
		for (x = 0; x < VORONOI_TEX_W; ++x) {
			unsigned int minDist = 127*128;
			for (i = 0; i < VORONOI_POINTS; ++i) {
				Point2D* p = &points[i];
				int px = (p->x - x);
				int py = (p->y - y);
				unsigned int dist = px * px + py * py;
				//int px = p->x & (VORONOI_TEX_W - 1);
				//int py = p->y & (VORONOI_TEX_H - 1);
				//unsigned int dist = (px - x) * (px - x) + (py - y) * (py - y);
				if (dist < minDist) minDist = dist;
			}
			c = minDist >> (7 - 3);
			if (c > 63) c = 63;
			*dst++ = c;
		}
	}
}

static void renderVoronoiTexture()
{
	int x,y;

	unsigned char *src = texture;
	uint16_t *dst = fb_pixels;
	for (y = 0; y < VORONOI_TEX_H; ++y) {
		for (x = 0; x < VORONOI_TEX_W; ++x) {
			int c = *src++;
			int r = c >> 1;
			int g = r;
			int b = r;
			*(dst+x) = (r << 11) | (g << 5) | b;
		}
		dst += FB_WIDTH;
	}
}

static void voronoi_draw(void)
{
	memset16(fb_pixels, PACK_RGB16(144, 96, 32), 320 * 240);

	updateVoronoiPoints();

	updateVoronoiTexture();

	renderVoronoiTexture();

	swap_buffers(fb_pixels);
}

static void voronoi_keypress(int key)
{
	switch(key) {
	case ' ':
		break;

	default:
		break;
	}
}
