#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psys.h"
#include "3dgfx/3dgfx.h"
#include "util.h"

static void pupdate(struct ps_emitter *ps, struct ps_part *p, float dt);
static void pspawn(struct ps_emitter *ps);
static INLINE void drawpart(struct ps_emitter *ps, struct ps_part *p);
static INLINE float frand(void);

int ps_init(struct ps_emitter *ps)
{
	memset(ps, 0, sizeof *ps);

	ps->life = 1.0f;
	ps->spawn_rate = 10.0f;
	ps->size0 = ps->size1 = 1.0f;
	cgm_wcons(&ps->col0, 1, 1, 1, 1);
	cgm_wcons(&ps->col1, 0, 0, 0, 0);
	ps->grav = -9.8f;

	return 0;
}

int ps_init_from(struct ps_emitter *ps, const struct ps_emitter *proto)
{
	memcpy(ps, proto, sizeof *ps);
	ps->plist = 0;
	ps->pcount = 0;
	ps->spawn_acc = 0;
	return 0;
}

void ps_destroy(struct ps_emitter *ps)
{
	struct ps_pblock *blk;

	while(ps->plist) {
		blk = ps->plist;
		ps->plist = blk->next;
		ps_free_block(blk);
	}
}

void ps_update(struct ps_emitter *ps, float dt)
{
	unsigned int i, alive;
	struct ps_pblock *blk, *prev, dummy;
	struct ps_part *p;
	void *tmp;

	/* step any existing particles */
	if(!ps->plist) goto skip_existing;

	blk = ps->plist;
	while(blk) {
		if(!blk->alive) goto next;
		alive = blk->alive;

		for(i=0; i<32; i++) {
			if(alive & 1) {
				p = blk->p + i;
				pupdate(ps, p, dt);
				if(p->tm >= ps->life) {
					blk->alive &= ~(1 << i);
				}
			}
			alive >>= 1;
		}

next:	blk = blk->next;
	}
skip_existing:

	/* spawn new particles */
	ps->spawn_acc += ps->spawn_rate * dt;
	while(ps->spawn_acc >= 1.0f) {
		pspawn(ps);
		ps->spawn_acc -= 1.0f;
	}

	/* free any empty blocks */
	dummy.next = ps->plist;
	prev = &dummy;

	while((blk = prev->next)) {
		if(!blk->alive) {
			prev->next = blk->next;
			ps_free_block(blk);
		} else {
			prev = blk;
		}
	}
	ps->plist = dummy.next;
}

#define DAMPING		0.001

static void pupdate(struct ps_emitter *ps, struct ps_part *p, float dt)
{
	float t;
	cgm_vec4 col;

	p->p.x += p->v.x * dt;
	p->p.y += p->v.y * dt;
	p->p.z += p->v.z * dt;

	p->v.x -= p->v.x * DAMPING * dt;
	p->v.y -= p->v.y * DAMPING * dt;
	p->v.z -= p->v.z * DAMPING * dt;

	p->v.y += ps->grav * dt;

	p->tm += dt;
	if(p->tm >= ps->life) {
		t = 1.0f;
	} else {
		t = p->tm / ps->life;
	}

	p->sz = cgm_lerp(ps->size0, ps->size1, t);
	cgm_wlerp(&col, &ps->col0, &ps->col1, t);

	p->r = cround64(col.x * col.w * 255.0f);
	p->g = cround64(col.y * col.w * 255.0f);
	p->b = cround64(col.z * col.w * 255.0f);
}

static struct ps_part *palloc(struct ps_emitter *ps)
{
	unsigned int i;
	struct ps_pblock *blk;
	struct ps_part *p;

	blk = ps->plist;
	while(blk) {
		if(blk->alive == 0xffffffff) {
			blk = blk->next;
			continue;
		}
		for(i=0; i<32; i++) {
			if((blk->alive & (1 << i)) == 0) {
				blk->alive |= 1 << i;
				return blk->p + i;
			}
		}
		blk = blk->next;
	}

	/* no empty spot, get a new block */
	if(!(blk = ps_alloc_block())) {
		return 0;
	}
	blk->next = ps->plist;
	ps->plist = blk;
	blk->alive = 1;
	return blk->p;
}

static void pspawn(struct ps_emitter *ps)
{
	struct ps_part *p;
	cgm_vec4 col;

	if(!(p = palloc(ps))) return;

	p->p = ps->pos;
	if(ps->pos_rad) {
		p->p.x += frand() * ps->pos_rad * 2.0f - ps->pos_rad;
		p->p.y += frand() * ps->pos_rad * 2.0f - ps->pos_rad;
		p->p.z += frand() * ps->pos_rad * 2.0f - ps->pos_rad;
	}

	p->v = ps->vel;
	if(ps->vel_rad) {
		p->v.x += frand() * ps->vel_rad * 2.0f - ps->vel_rad;
		p->v.y += frand() * ps->vel_rad * 2.0f - ps->vel_rad;
		p->v.z += frand() * ps->vel_rad * 2.0f - ps->vel_rad;
	}

	p->sz = ps->size0;

	p->r = cround64(ps->col0.x * 255.0f);
	p->g = cround64(ps->col0.y * 255.0f);
	p->b = cround64(ps->col0.z * 255.0f);

	p->tm = 0;
}

void ps_draw(struct ps_emitter *ps)
{
	unsigned int i, sav_en, sav_pmode;
	uint32_t alive;
	struct ps_pblock *blk;

	sav_en = g3d_getopt(0xffffffff);
	sav_pmode = g3d_get_polygon_mode();

	g3d_disable(G3D_LIGHTING);
	g3d_enable(G3D_ADD_BLEND);
	g3d_polygon_mode(G3D_FLAT);
	g3d_depth_mask(0);

	blk = ps->plist;
	while(blk) {
		if(!blk->alive) goto next;

		alive = blk->alive;
		for(i=0; i<32; i++) {
			if(alive & 1) {
				drawpart(ps, blk->p + i);
			}
			alive >>= 1;
		}
next:	blk = blk->next;
	}

	g3d_setopt(sav_en, 0xffffffff);
	g3d_polygon_mode(sav_pmode);
	g3d_depth_mask(1);
}

static INLINE void drawpart(struct ps_emitter *ps, struct ps_part *p)
{
	draw_billboard(p->p.x, p->p.y, p->p.z, p->sz, p->r, p->g, p->b, 255);
}

static INLINE float frand(void)
{
	return (float)rand() / (float)RAND_MAX;
}

/* particle block allocator */
#define MAX_BLOCKS	256

static struct ps_pblock *ppool;
static unsigned int total_blk;
static unsigned int blk_limited;

struct ps_pblock *ps_alloc_block(void)
{
	struct ps_pblock *blk;
	if(ppool) {
		blk = ppool;
		ppool = ppool->next;
		return blk;
	}

	/* no blocks in the pool, alloc more if we haven't reached the limit */	
	if(total_blk >= MAX_BLOCKS) {
		if(!blk_limited) {
			blk_limited = 1;
			fprintf(stderr, "warning: particle block limit exceeded\n");
			return 0;
		}
	}

	if(!(blk = malloc(sizeof *blk))) {
		fprintf(stderr, "failed to allocate particle block\n");
		return 0;
	}
	blk->alive = 0;
	blk->next = 0;
	return blk;
}

void ps_free_block(struct ps_pblock *blk)
{
	blk->next = ppool;
	ppool = blk;
}
