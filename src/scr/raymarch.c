#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "screen.h"
#include "demo.h"
#include "3dgfx.h"
#include "util.h"
#include "mesh.h"
#include "cgmath/cgmath.h"

static int raymarch_init(void);
static void raymarch_destroy(void);
static void raymarch_start(long trans_time);
static void raymarch_stop(long trans_time);
static void raymarch_draw(void);
static void raymarch_keypress(int key);

static struct screen scr = {
	"raymarch",
	raymarch_init, raymarch_destroy,
	raymarch_start, raymarch_stop,
	raymarch_draw,
	raymarch_keypress
};

struct screen* raymarch_screen(void)
{
	return &scr;
}

static int raymarch_init(void)
{
	return 0;
}

static void raymarch_destroy(void)
{
}

static void raymarch_start(long trans_time)
{
}

static void raymarch_stop(long trans_time)
{
}

static void raymarch_draw(void)
{
	memset16(fb_pixels, PACK_RGB16(64, 128, 192), 320 * 240);

	swap_buffers(fb_pixels);
}

static void raymarch_keypress(int key)
{
	switch (key) {
	case ' ':
		break;

	default:
		break;
	}
}
