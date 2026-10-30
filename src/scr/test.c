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


static int test_init(void);
static void destroy(void);
static void start(long trans_time);
static void draw(void);
static void shade_blobs(void);

static struct screen scr = {
	"metaballs",
	test_init,
	destroy,
	start, 0,
	draw
};

static float cam_theta, cam_phi;
static float cam_dist = 8;


struct screen *test_screen(void)
{
	return &scr;
}

static int test_init(void)
{
	return 0;
}

static void destroy(void)
{
}

static void start(long trans_time)
{
}

static void update(void)
{
	mouse_orbit_update(&cam_theta, &cam_phi, &cam_dist);
}

static void draw(void)
{
	update();

	memset16(fb_pixels, 0xfc51, FB_WIDTH * FB_HEIGHT);

	swap_buffers(fb_pixels);
}
