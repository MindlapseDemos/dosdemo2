#ifndef PSYS_H_
#define PSYS_H_

#include "szint.h"
#include "cgmath/cgmath.h"

struct ps_part {
	cgm_vec3 p, v;
	unsigned int r, g, b;
	float sz;
	float tm;
};

struct ps_pblock {
	uint32_t alive;	/* bitmap */
	struct ps_part p[32];
	struct ps_pblock *next;
};

struct ps_emitter {
	cgm_vec3 pos;
	float pos_rad;
	cgm_vec3 vel;
	float vel_rad;
	float size0, size1;
	cgm_vec4 col0, col1;
	float grav;
	float life;

	float spawn_rate, spawn_acc;

	struct image *img;

	struct ps_pblock *plist;
	unsigned int pcount;
};

int ps_init(struct ps_emitter *ps);
int ps_init_from(struct ps_emitter *ps, const struct ps_emitter *proto);
void ps_destroy(struct ps_emitter *ps);

void ps_update(struct ps_emitter *ps, float dt);
void ps_draw(struct ps_emitter *ps);

struct ps_pblock *ps_alloc_block(void);
void ps_free_block(struct ps_pblock *blk);

#endif	/* PSYS_H_ */
