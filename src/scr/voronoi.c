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

#define VORONOI_TEX_W 64
#define VORONOI_TEX_H 64

#define VORONOI_GRID_SIZE_X 16
#define VORONOI_GRID_SIZE_Y 16

#define VORONOI_GRID_W (VORONOI_TEX_W / VORONOI_GRID_SIZE_X)
#define VORONOI_GRID_H (VORONOI_TEX_H / VORONOI_GRID_SIZE_Y)

#define VORONOI_BORDERS 4

#define VORONOI_POINTS_W (VORONOI_GRID_W + VORONOI_BORDERS)
#define VORONOI_POINTS_H (VORONOI_GRID_H + VORONOI_BORDERS)

#define VORONOI_POINTS 16

static Point2D points[VORONOI_POINTS];
static unsigned char texture[VORONOI_TEX_W * VORONOI_TEX_H];

static Point2D pointsList[VORONOI_POINTS_W * VORONOI_POINTS_H][VORONOI_POINTS];
static int pointsListCount[VORONOI_POINTS_W * VORONOI_POINTS_H];


struct screen * voronoi_screen(void)
{
	return &scr;
}

static int voronoi_init(void)
{
	memset(pointsList, 0, VORONOI_POINTS * VORONOI_POINTS_W * VORONOI_POINTS_H * sizeof(Point2D));
	memset(pointsListCount, 0, VORONOI_POINTS_W * VORONOI_POINTS_H * sizeof(int));

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
	float t2 = time_msec * 0.0005f;
	float t3 = time_msec * 0.0004f;
	float t4 = time_msec * 0.0002f;

	float sx = 1.25f;
	float sy = 1.25f;
	float dx1 = VORONOI_TEX_W * 0.275f * sx;
	float dx2 = VORONOI_TEX_W * 0.175f * sx;
	float dy1 = VORONOI_TEX_H * 0.25f * sy;
	float dy2 = VORONOI_TEX_H * 0.3f * sy;

	memset(pointsListCount, 0, VORONOI_POINTS_W * VORONOI_POINTS_H * sizeof(int));

	for (i = 0; i < VORONOI_POINTS; ++i) {
		int celX, celY;
		int celWrapX, celWrapY;
		int wrapX, wrapY;

		float it1 = (float)(i - 2) * 1.77f;
		float it2 = (float)(i + 3) * 2.36f;

		int x = VORONOI_TEX_W / 2 + (int)(sin(t1 + it1) * dx1 + sin(t2 + it2) * dx2);
		int y = VORONOI_TEX_H / 2 + (int)(sin(t3 - it2) * dy1 + sin(t4 + it1) * dy2);

		celX = x / VORONOI_GRID_SIZE_X;
		celY = y / VORONOI_GRID_SIZE_Y;

		if (x < 0) celX--;
		if (y < 0) celY--;

		if (celX >= -1 && celX <= VORONOI_GRID_W && celY >= -1 && celY <= VORONOI_GRID_H) {
			int gridCelOffset = (celY + VORONOI_BORDERS/2) * VORONOI_POINTS_W + celX + VORONOI_BORDERS / 2;
			Point2D* pCel = pointsList[gridCelOffset];
			int count = pointsListCount[gridCelOffset]++;
			pCel[count].x = x;
			pCel[count].y = y;

			wrapX = x;
			wrapY = y;
			celWrapX = celX;
			celWrapY = celY;
			if (celX == -1) {
				celWrapX = VORONOI_GRID_W;
				wrapX = x + VORONOI_GRID_SIZE_X * (VORONOI_GRID_W + 1);
			} else if (celX == VORONOI_GRID_W) {
				celWrapX = -1;
				wrapX = x - VORONOI_GRID_SIZE_X * (VORONOI_GRID_W + 1);
			}
			if (celY == -1) {
				celWrapY = VORONOI_GRID_H;
				wrapY = y + VORONOI_GRID_SIZE_Y * (VORONOI_GRID_H + 1);
			}
			else if (celY == VORONOI_GRID_H) {
				celWrapY = -1;
				wrapY = y - VORONOI_GRID_SIZE_Y * (VORONOI_GRID_H + 1);
			}

			// add duplicate point on other side for later?
			if (celWrapX != celX || celWrapY != celY) {
				gridCelOffset = (celWrapY + VORONOI_BORDERS / 2) * VORONOI_POINTS_W + celWrapX + VORONOI_BORDERS / 2;
				pCel = pointsList[gridCelOffset];
				count = pointsListCount[gridCelOffset]++;
				pCel[count].x = wrapX;
				pCel[count].y = wrapY;
			}
		}
	}
}

static void updateVoronoiTexture()
{
	int x, y, i, j, k;
	unsigned int c;

	unsigned char *dst = texture;
	for (y = 0; y < VORONOI_TEX_H; ++y) {
		int celY = (y - VORONOI_GRID_SIZE_Y / 2) / VORONOI_GRID_SIZE_Y;
		int gridCelOffsetY = (celY + VORONOI_BORDERS / 2) * VORONOI_POINTS_W;
		for (x = 0; x < VORONOI_TEX_W; ++x) {
			int celX = (x - VORONOI_GRID_SIZE_X / 2) / VORONOI_GRID_SIZE_X;
			int gridCelOffset = gridCelOffsetY + celX + VORONOI_BORDERS / 2;

			unsigned int minDist = 127 * 128;

			for (k = -2; k < 3; ++k) {
				int gridCelQuadrantOffset = gridCelOffset + k * VORONOI_POINTS_W;
				for (j = -2; j < 3; ++j) {
					int count = pointsListCount[gridCelQuadrantOffset + j];
					Point2D* pCel = pointsList[gridCelQuadrantOffset + j];
					for (i = 0; i < count; ++i) {
						int px = (pCel->x - x);
						int py = (pCel->y - y);
						unsigned int dist = px * px + py * py;
						if (dist < minDist) minDist = dist;
						pCel++;
					}
				}
			}

			c = (minDist >> (7 - 4)) & 127;
			if (c > 63) c = 127 - c;
			*dst++ = c;
		}
	}
}

/*static void renderVoronoiPoints()
{
	int x, y, i;

	int offX = 64;
	int offY = 64;

	uint16_t* dst = fb_pixels;
	for (x = offX; x < VORONOI_TEX_W + offX; ++x) {
		*(dst + offY * FB_WIDTH + x) = 0xffffffff;
		*(dst + (offY + VORONOI_TEX_H-1) * FB_WIDTH + x) = 0xffffffff;
	}
	for (y = offY; y < VORONOI_TEX_H + offY; ++y) {
		*(dst + y * FB_WIDTH + offX) = 0xffffffff;
		*(dst + y * FB_WIDTH + offX + VORONOI_TEX_W - 1) = 0xffffffff;
	}

	for (y = 0; y < VORONOI_POINTS_H; ++y) {
		for (x = 0; x < VORONOI_POINTS_W; ++x) {
			int offset = y * VORONOI_POINTS_W + x;
			int count = pointsListCount[offset];
			Point2D* pCel = pointsList[offset];

			for (i = 0; i < count; ++i) {
				int xp = pCel[i].x + offX;
				int yp = pCel[i].y + offY;

				if (xp >= 0 && xp < FB_WIDTH && yp >= 0 && yp < FB_HEIGHT) {
					*(dst + yp * FB_WIDTH + xp) = 0xffff0000;
				}
			}
		}
	}
}*/

static void renderVoronoiTextureRepeat()
{
	int x, y;

	unsigned char* src = texture;
	uint16_t* dst = fb_pixels;
	for (y = 0; y < FB_HEIGHT; ++y) {
		unsigned char* src = &texture[(y & (VORONOI_TEX_H - 1)) * VORONOI_TEX_W];
		for (x = 0; x < FB_WIDTH; ++x) {
			int c = *(src + (x & (VORONOI_TEX_W - 1)));
			int r = c >> 1;
			int g = r;
			int b = r;
			*(dst + x) = (r << 11) | (g << 5) | b;
		}
		dst += FB_WIDTH;
	}
}

static void renderVoronoiTextureMirror()
{
	int x, y;

	unsigned char* src = texture;
	uint16_t* dst = fb_pixels;
	for (y = 0; y < VORONOI_TEX_H; ++y) {
		for (x = 0; x < VORONOI_TEX_W; ++x) {
			int c = *src++;
			int r = c >> 1;
			int g = r;
			int b = r;
			uint16_t c16 = (r << 11) | (g << 5) | b;
			*(dst + y * FB_WIDTH + x) = c16;
			*(dst + y * FB_WIDTH + 2 * VORONOI_TEX_W - x - 1) = c16;
			*(dst + (2 * VORONOI_TEX_H - y - 1) * FB_WIDTH + x) = c16;
			*(dst + (2 * VORONOI_TEX_H - y - 1) * FB_WIDTH + 2 * VORONOI_TEX_W - x - 1) = c16;
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

	//renderVoronoiTexture();
	//renderVoronoiTextureRepeat();
	renderVoronoiTextureMirror();
	//renderVoronoiPoints();

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
