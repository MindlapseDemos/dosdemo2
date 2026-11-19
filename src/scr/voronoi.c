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

struct screen * voronoi_screen(void)
{
	return &scr;
}

static int voronoi_init(void)
{
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

static void voronoi_draw(void)
{
	memset16(fb_pixels, PACK_RGB16(144, 96, 32), 320 * 240);

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
