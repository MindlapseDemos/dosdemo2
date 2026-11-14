#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mesh.h"
#include "3dgfx.h"
#include "util.h"
#include "dynarr.h"
#include "cgmath/cgmath.h"

void init_g3dmtl(struct g3d_material *mtl)
{
	mtl->name = 0;
	mtl->r = mtl->g = mtl->b = mtl->a = 1.0f;
	mtl->sr = mtl->sg = mtl->sb = 0.0f;
	mtl->shin = 60.0f;
	mtl->texmap = mtl->envmap = 0;
}

int init_mesh(struct g3d_mesh *mesh, int prim, int num_verts, int num_idx)
{
	memset(mesh, 0, sizeof *mesh);
	mesh->prim = prim;
	if(num_verts > 0) {
		mesh->varr = calloc_nf(num_verts, sizeof *mesh->varr);
	} else {
		mesh->varr = 0;
	}
	if(num_idx > 0) {
		mesh->iarr = calloc_nf(num_idx * prim, sizeof *mesh->iarr);
	} else {
		mesh->iarr = 0;
	}
	mesh->vcount = num_verts;
	mesh->icount = num_idx;
	return 0;
}

void free_mesh(struct g3d_mesh *mesh)
{
	destroy_mesh(mesh);
	free(mesh);
}

void destroy_mesh(struct g3d_mesh *mesh)
{
	if(mesh) {
		free(mesh->name);
		free(mesh->varr);
		free(mesh->iarr);
		free(mesh->vprim);
	}
}

int copy_mesh(struct g3d_mesh *dest, struct g3d_mesh *src)
{
	dest->prim = src->prim;
	if(src->varr) {
		if(!(dest->varr = malloc(src->vcount * sizeof *src->varr))) {
			return -1;
		}
		memcpy(dest->varr, src->varr, src->vcount * sizeof *src->varr);
	}
	dest->vcount = src->vcount;
	if(src->iarr) {
		if(!(dest->iarr = malloc(src->icount * sizeof *src->iarr))) {
			free(dest->varr);
			dest->varr = 0;
			return -1;
		}
		memcpy(dest->iarr, src->iarr, src->icount * sizeof *src->iarr);
	}
	dest->icount = src->icount;
	dest->flags = src->flags;
	return 0;
}

static struct {
	int prim;
	struct g3d_vertex *varr;
	const float *xform;
} zsort_cls;

static int zsort_cmp(const void *aptr, const void *bptr)
{
	int i;
	float za = 0.0f;
	float zb = 0.0f;
	const float *m = zsort_cls.xform;
	const struct g3d_vertex *va = (const struct g3d_vertex*)aptr;
	const struct g3d_vertex *vb = (const struct g3d_vertex*)bptr;

	for(i=0; i<zsort_cls.prim; i++) {
		za += m[2] * va->x + m[6] * va->y + m[10] * va->z + m[14];
		zb += m[2] * vb->x + m[6] * vb->y + m[10] * vb->z + m[14];
		++va;
		++vb;
	}

	za -= zb;
	return *(int*)&za;
}

static int zsort_indexed_cmp(const void *aptr, const void *bptr)
{
	int i;
	float za = 0.0f;
	float zb = 0.0f;
	const uint16_t *a = (const uint16_t*)aptr;
	const uint16_t *b = (const uint16_t*)bptr;

	const float *m = zsort_cls.xform;

	for(i=0; i<zsort_cls.prim; i++) {
		const struct g3d_vertex *va = zsort_cls.varr + a[i];
		const struct g3d_vertex *vb = zsort_cls.varr + b[i];

		za += m[2] * va->x + m[6] * va->y + m[10] * va->z + m[14];
		zb += m[2] * vb->x + m[6] * vb->y + m[10] * vb->z + m[14];
	}

	za -= zb;
	return *(int*)&za;
}


void zsort_mesh(struct g3d_mesh *m)
{
	zsort_cls.varr = m->varr;
	zsort_cls.xform = g3d_get_matrix(G3D_MODELVIEW, 0);
	zsort_cls.prim = m->prim;

	if(m->iarr) {
		int nfaces = m->icount / m->prim;
		qsort(m->iarr, nfaces, m->prim * sizeof *m->iarr, zsort_indexed_cmp);
	} else {
		int nfaces = m->vcount / m->prim;
		qsort(m->varr, nfaces, m->prim * sizeof *m->varr, zsort_cmp);
	}
}


void draw_mesh(struct g3d_mesh *mesh)
{
	struct g3d_material *mtl;

	if((mtl = mesh->mtl)) {
		g3d_mtl_diffuse(mtl->r, mtl->g, mtl->b);
		g3d_mtl_specular(mtl->sr, mtl->sg, mtl->sb);
		g3d_mtl_shininess(mtl->shin);

		if(mtl->texmap) {
			g3d_enable(G3D_TEXTURE_2D);
			g3d_set_texture(mtl->texmap->width, mtl->texmap->height, mtl->texmap->pixels);
		} else if(mtl->envmap) {
			g3d_enable(G3D_TEXTURE_2D);
			g3d_set_texture(mtl->envmap->width, mtl->envmap->height, mtl->envmap->pixels);
			g3d_enable(G3D_TEXTURE_GEN);
		}
	}

	if(mesh->iarr) {
		g3d_draw_indexed(mesh->prim, mesh->varr, mesh->vcount, mesh->iarr, mesh->icount);
	} else {
		g3d_draw(mesh->prim, mesh->varr, mesh->vcount);
	}

	if(mtl) {
		if(mtl->texmap) {
			g3d_disable(G3D_TEXTURE_2D);
		} else if(mtl->envmap) {
			g3d_disable(G3D_TEXTURE_2D);
			g3d_disable(G3D_TEXTURE_GEN);
		}
	}
}

void apply_mesh_xform(struct g3d_mesh *mesh, const float *xform)
{
	int i;
	struct g3d_vertex *v = mesh->varr;

	for(i=0; i<mesh->vcount; i++) {
		float x = xform[0] * v->x + xform[4] * v->y + xform[8] * v->z + xform[12];
		float y = xform[1] * v->x + xform[5] * v->y + xform[9] * v->z + xform[13];
		v->z = xform[2] * v->x + xform[6] * v->y + xform[10] * v->z + xform[14];
		v->x = x;
		v->y = y;
		x = xform[0] * v->nx + xform[4] * v->ny + xform[8] * v->nz;
		y = xform[1] * v->nx + xform[5] * v->ny + xform[9] * v->nz;
		v->nz = xform[2] * v->nx + xform[6] * v->ny + xform[10] * v->nz;
		v->nx = x;
		v->ny = y;
		++v;
	}
}

int append_mesh(struct g3d_mesh *ma, struct g3d_mesh *mb)
{
	int i, new_vcount, new_icount;
	void *tmp;
	uint16_t *iptr;

	if(ma->prim != mb->prim) {
		fprintf(stderr, "append_mesh failed, primitive mismatch\n");
		return -1;
	}

	if(ma->iarr || mb->iarr) {
		if(!ma->iarr) {
			if(indexify_mesh(ma) == -1) {
				return -1;
			}
		} else if(!mb->iarr) {
			if(indexify_mesh(mb) == -1) {
				return -1;
			}
		}

		new_icount = ma->icount + mb->icount;
		if(!(iptr = realloc(ma->iarr, new_icount * sizeof *iptr))) {
			fprintf(stderr, "append_mesh: failed to allocate combined index buffer (%d indices)\n", new_icount);
			return -1;
		}
		ma->iarr = iptr;

		iptr += ma->icount;
		for(i=0; i<mb->icount; i++) {
			*iptr++ = mb->iarr[i] + ma->vcount;
		}
		ma->icount = new_icount;
	}

	new_vcount = ma->vcount + mb->vcount;
	if(!(tmp = realloc(ma->varr, new_vcount * sizeof *ma->varr))) {
		fprintf(stderr, "append_mesh: failed to allocate combined vertex buffer (%d verts)\n", new_vcount);
		return -1;
	}
	ma->varr = tmp;
	memcpy(ma->varr + ma->vcount, mb->varr, mb->vcount * sizeof *ma->varr);
	ma->vcount = new_vcount;
	return 0;
}

#define FEQ(a, b)	((a) - (b) < 1e-5 && (b) - (a) < 1e-5)
static int cmp_vertex(struct g3d_vertex *a, struct g3d_vertex *b)
{
	if(!FEQ(a->x, b->x) || !FEQ(a->y, b->y) || !FEQ(a->z, b->z) || !FEQ(a->w, b->w))
		return -1;
	if(!FEQ(a->nx, b->nx) || !FEQ(a->ny, b->ny) || !FEQ(a->nz, b->nz))
		return -1;
	if(!FEQ(a->u, b->u) || !FEQ(a->v, b->v))
		return -1;
	if(a->r != b->r || a->g != b->g || a->b != b->b || a->a != b->a)
		return -1;
	return 0;
}

static int find_existing(struct g3d_vertex *v, struct g3d_vertex *varr, int vcount)
{
	int i;
	for(i=0; i<vcount; i++) {
		if(cmp_vertex(v, varr++) == 0) {
			return i;
		}
	}
	return -1;
}

int indexify_mesh(struct g3d_mesh *mesh)
{
	int i, j, nfaces, max_icount, idx;
	int out_vcount = 0;
	struct g3d_vertex *vin, *vout;
	uint16_t *iout;

	if(mesh->iarr) {
		fprintf(stderr, "indexify_mesh failed: already indexed\n");
		return -1;
	}

	nfaces = mesh->vcount / mesh->prim;
	max_icount = nfaces * mesh->prim;

	if(!(mesh->iarr = malloc(max_icount * sizeof *mesh->iarr))) {
		fprintf(stderr, "indexify_mesh failed to allocate index buffer of %d indices\n", max_icount);
		return -1;
	}

	vin = vout = mesh->varr;
	iout = mesh->iarr;

	for(i=0; i<nfaces; i++) {
		for(j=0; j<mesh->prim; j++) {
			if((idx = find_existing(vin, mesh->varr, out_vcount)) >= 0) {
				*iout++ = idx;
			} else {
				*iout++ = out_vcount++;
				if(vin != vout) {
					*vout = *vin;
				}
				++vout;
			}
			++vin;
		}
	}

	mesh->vcount = out_vcount;
	mesh->icount = max_icount;
	/* XXX also shrink buffers? I'll just leave them to max size for now */
	return 0;
}

void normalize_mesh_normals(struct g3d_mesh *mesh)
{
	int i;
	struct g3d_vertex *v = mesh->varr;

	for(i=0; i<mesh->vcount; i++) {
		float mag = sqrt(v->nx * v->nx + v->ny * v->ny + v->nz * v->nz);
		float s = (mag == 0.0f) ? 1.0f : 1.0f / mag;
		v->nx *= s;
		v->ny *= s;
		v->nz *= s;
		++v;
	}
}


void calc_mesh_centroid(struct g3d_mesh *mesh, float *cent)
{
	int i;
	float s = 1.0f / (float)mesh->vcount;
	cent[0] = cent[1] = cent[2] = 0.0f;

	for(i=0; i<mesh->vcount; i++) {
		cent[0] += mesh->varr[i].x;
		cent[1] += mesh->varr[i].y;
		cent[2] += mesh->varr[i].z;
	}
	cent[0] *= s;
	cent[1] *= s;
	cent[2] *= s;
}

static int vert_same(struct g3d_vertex *a, struct g3d_vertex *b)
{
	float dx = fabs(a->x - b->x);
	float dy = fabs(a->y - b->y);
	float dz = fabs(a->z - b->z);
	float max = dx > dy ? (dx > dz ? dx : dz) : (dy > dz ? dy : dz);
	return max < 1e-4;
}

static int prepare_vprim(struct g3d_mesh *mesh)
{
	int i, j;

	if(mesh->vprim) return 0;

	if(!(mesh->vprim = malloc(mesh->vcount * sizeof *mesh->vprim))) {
		fprintf(stderr, "failed to allocate vertex identity table\n");
		return -1;
	}

	for(i=0; i<mesh->vcount; i++) {
		mesh->vprim[i] = i;
		for(j=0; j<i; j++) {
			if(vert_same(mesh->varr + i, mesh->varr + j)) {
				mesh->vprim[i] = j;
			}
		}
	}
	return 0;
}

void calc_mesh_normals(struct g3d_mesh *mesh)
{
	int i, j, nfaces;
	uint16_t *idxptr, vidx;
	cgm_vec3 ab, ac, n;
	struct g3d_vertex *va, *vb, *vc, *vert, *vptr;

	if(!mesh->vprim && prepare_vprim(mesh) == -1) {
		return;
	}

	vert = mesh->varr;
	for(i=0; i<mesh->vcount; i++) {
		vert->nx = vert->ny = vert->nz = 0.0f;
		vert++;
	}

	if(mesh->iarr) {
		nfaces = mesh->icount / mesh->prim;

		idxptr = mesh->iarr;
		for(i=0; i<nfaces; i++) {
			va = mesh->varr + idxptr[0];
			vb = mesh->varr + idxptr[1];
			vc = mesh->varr + idxptr[2];

			cgm_vcons(&ab, vb->x - va->x, vb->y - va->y, vb->z - va->z);
			cgm_vcons(&ac, vc->x - va->x, vc->y - va->y, vc->z - va->z);
			cgm_vcross(&n, &ab, &ac);

			for(j=0; j<mesh->prim; j++) {
				vidx = mesh->vprim[idxptr[j]];
				vptr = mesh->varr + vidx;
				vptr->nx += n.x;
				vptr->ny += n.y;
				vptr->nz += n.z;
			}
			idxptr += mesh->prim;
		}

	} else {
		nfaces = mesh->vcount / mesh->prim;

		vert = mesh->varr;
		for(i=0; i<nfaces; i++) {
			cgm_vcons(&ab, vert[1].x - vert[0].x, vert[1].y - vert[0].y, vert[1].z - vert[0].z);
			cgm_vcons(&ac, vert[2].x - vert[0].x, vert[2].y - vert[0].y, vert[2].z - vert[0].z);
			cgm_vcross(&n, &ab, &ac);

			for(j=0; j<mesh->prim; j++) {
				vidx = mesh->vprim[vert + j - mesh->varr];
				vptr = mesh->varr + vidx;
				vptr->nx += n.x;
				vptr->ny += n.y;
				vptr->nz += n.z;
			}
			vert += mesh->prim;
		}
	}

	vert = mesh->varr;
	for(i=0; i<mesh->vcount; i++) {
		if(mesh->vprim[i] != i) {
			vptr = mesh->varr + mesh->vprim[i];
			vert->nx = vptr->nx;
			vert->ny = vptr->ny;
			vert->nz = vptr->nz;
		}
		cgm_vnormalize((cgm_vec3*)&vert->nx);
		vert++;
	}
}

static void sphvec(float *res, float theta, float phi, float rad)
{
	theta = -theta;
	res[0] = sin(theta) * sin(phi) * rad;
	res[1] = cos(phi) * rad;
	res[2] = cos(theta) * sin(phi) * rad;
}

int gen_sphere_mesh(struct g3d_mesh *mesh, float rad, int usub, int vsub)
{
	int i, j;
	int nfaces, uverts, vverts;
	struct g3d_vertex *vptr;
	uint16_t *iptr;

	init_mesh(mesh, G3D_QUADS, 0, 0);

	if(usub < 4) usub = 4;
	if(vsub < 2) vsub = 2;

	uverts = usub + 1;
	vverts = vsub + 1;

	mesh->vcount = uverts * vverts;
	nfaces = usub * vsub;
	mesh->icount = nfaces * 4;

	if(!(mesh->varr = malloc(mesh->vcount * sizeof *mesh->varr))) {
		fprintf(stderr, "gen_sphere_mesh: failed to allocate vertex buffer (%d vertices)\n", mesh->vcount);
		return -1;
	}
	if(!(mesh->iarr = malloc(mesh->icount * sizeof *mesh->iarr))) {
		fprintf(stderr, "gen_sphere_mesh: failed to allocate index buffer (%d indices)\n", mesh->icount);
		return -1;
	}
	vptr = mesh->varr;
	iptr = mesh->iarr;

	for(i=0; i<uverts; i++) {
		float u = (float)i / (float)(uverts - 1);
		float theta = u * 2.0 * M_PI;

		for(j=0; j<vverts; j++) {
			float v = (float)j / (float)(vverts - 1);
			float phi = v * M_PI;
			int chess = (i & 1) == (j & 1);

			sphvec(&vptr->x, theta, phi, rad);
			vptr->w = 1.0f;

			vptr->nx = vptr->x / rad;
			vptr->ny = vptr->y / rad;
			vptr->nz = vptr->z / rad;
			vptr->u = u;
			vptr->v = v;
			vptr->r = chess ? 255 : 64;
			vptr->g = 128;
			vptr->b = chess ? 64 : 255;
			++vptr;

			if(i < usub && j < vsub) {
				int idx = i * vverts + j;
				*iptr++ = idx;
				*iptr++ = idx + vverts;
				*iptr++ = idx + vverts + 1;
				*iptr++ = idx + 1;
			}
		}
	}
	return 0;
}

#define PHI		1.618034
static cgm_vec3 icosa_pt[] = {
	{PHI, 1, 0},
	{-PHI, 1, 0},
	{PHI, -1, 0},
	{-PHI, -1, 0},
	{1, 0, PHI},
	{1, 0, -PHI},
	{-1, 0, PHI},
	{-1, 0, -PHI},
	{0, PHI, 1},
	{0, -PHI, 1},
	{0, PHI, -1},
	{0, -PHI, -1}
};
enum { P11, P12, P13, P14, P21, P22, P23, P24, P31, P32, P33, P34 };
static int icosa_idx[] = {
	P11, P31, P21,
	P11, P22, P33,
	P13, P21, P32,
	P13, P34, P22,
	P12, P23, P31,
	P12, P33, P24,
	P14, P32, P23,
	P14, P24, P34,

	P11, P33, P31,
	P12, P31, P33,
	P13, P32, P34,
	P14, P34, P32,

	P21, P13, P11,
	P22, P11, P13,
	P23, P12, P14,
	P24, P14, P12,

	P31, P23, P21,
	P32, P21, P23,
	P33, P22, P24,
	P34, P24, P22
};

static cgm_vec3 *geosphere(cgm_vec3 *verts, cgm_vec3 *v1, cgm_vec3 *v2, cgm_vec3 *v3, int iter)
{
	cgm_vec3 v12, v23, v31;

	if(!iter) {
		dynarr_push_nf(verts, v1);
		dynarr_push_nf(verts, v2);
		dynarr_push_nf(verts, v3);
		return verts;
	}

	v12 = *v1;
	cgm_vadd(&v12, v2);
	cgm_vnormalize(&v12);
	v23 = *v2;
	cgm_vadd(&v23, v3);
	cgm_vnormalize(&v23);
	v31 = *v3;
	cgm_vadd(&v31, v1);
	cgm_vnormalize(&v31);

	verts = geosphere(verts, v1, &v12, &v31, iter - 1);
	verts = geosphere(verts, v2, &v23, &v12, iter - 1);
	verts = geosphere(verts, v3, &v31, &v23, iter - 1);
	verts = geosphere(verts, &v12, &v23, &v31, iter - 1);
	return verts;
}

int gen_geosphere_mesh(struct g3d_mesh *mesh, float rad, int subdiv, int hemi)
{
	int i, j, num_verts, num_tri, vidx;
	cgm_vec3 v[3], *verts;
	struct g3d_vertex *varr, v0, v1;
	float theta, phi;

	num_tri = (sizeof icosa_idx / sizeof *icosa_idx) / 3;

	verts = dynarr_alloc_nf(0, sizeof *verts);
	for(i=0; i<num_tri; i++) {
		for(j=0; j<3; j++) {
			vidx = icosa_idx[i * 3 + j];
			v[j] = icosa_pt[vidx];
			cgm_vnormalize(v + j);
		}

		if(hemi && (v[0].y < 0.0 || v[1].y < 0.0 || v[2].y < 0.0)) {
			continue;
		}

		verts = geosphere(verts, v, v + 1, v + 2, subdiv);
	}

	num_verts = dynarr_size(verts);
	init_mesh(mesh, G3D_TRIANGLES, num_verts, 0);

	varr = mesh->varr;
	for(i=0; i<num_verts; i++) {
		varr->x = verts[i].x * rad;
		varr->y = verts[i].y * rad;
		varr->z = verts[i].z * rad;
		varr->w = 1.0f;

		varr->nx = verts[i].x;
		varr->ny = verts[i].y;
		varr->nz = verts[i].z;

		theta = atan2(verts[i].z, verts[i].x);
		phi = acos(verts[i].y);

		varr->u = 0.5 * theta / M_PI + 0.5;
		varr->v = phi / M_PI;
		varr++;
	}

	dynarr_free(verts);

	indexify_mesh(mesh);
	return 0;
}

int gen_plane_mesh(struct g3d_mesh *m, float width, float height, int usub, int vsub)
{
	int i, j;
	int nfaces, nverts, nidx, uverts, vverts;
	float x, y, u, v, du, dv;
	struct g3d_vertex *vptr;
	uint16_t *iptr;

	init_mesh(m, G3D_QUADS, 0, 0);

	if(usub < 1) usub = 1;
	if(vsub < 1) vsub = 1;

	nfaces = usub * vsub;
	uverts = usub + 1;
	vverts = vsub + 1;
	du = 1.0f / (float)usub;
	dv = 1.0f / (float)vsub;

	nverts = uverts * vverts;
	nidx = nfaces * 4;

	if(!(m->varr = malloc(nverts * sizeof *m->varr))) {
		fprintf(stderr, "gen_plane_mesh: failed to allocate vertex buffer (%d vertices)\n", nverts);
		return -1;
	}
	if(!(m->iarr = malloc(nidx * sizeof *m->iarr))) {
		fprintf(stderr, "gen_plane_mesh: failed to allocate index buffer (%d indices)\n", nidx);
		free(m->varr);
		m->varr = 0;
		return -1;
	}

	m->vcount = nverts;
	m->icount = nidx;

	vptr = m->varr;
	iptr = m->iarr;

	v = 0.0f;
	for(i=0; i<vverts; i++) {
		y = (v - 0.5) * height;
		u = 0.0f;

		for(j=0; j<uverts; j++) {
			x = (u - 0.5) * width;

			vptr->x = x;
			vptr->y = y;
			vptr->z = 0.0f;
			vptr->w = 1.0f;
			vptr->nx = 0.0f;
			vptr->ny = 0.0f;
			vptr->nz = 1.0f;
			vptr->u = u;
			vptr->v = v;
			vptr->r = vptr->g = vptr->b = vptr->a = 255;
			++vptr;

			u += du;
		}
		v += dv;
	}

	for(i=0; i<vsub; i++) {
		for(j=0; j<usub; j++) {
			int idx = i * uverts + j;
			*iptr++ = idx;
			*iptr++ = idx + 1;
			*iptr++ = idx + uverts + 1;
			*iptr++ = idx + uverts;
		}
	}
	return 0;
}

int gen_cube_mesh(struct g3d_mesh *mesh, float sz, int sub)
{
	int i;
	float offs;
	struct g3d_mesh *m;
	struct g3d_mesh tmpmesh;
	static float rotface[][4] = {
		{0, 0, 1, 0},
		{90, 0, 1, 0},
		{180, 0, 1, 0},
		{270, 0, 1, 0},
		{90, 1, 0, 0},
		{-90, 1, 0, 0}
	};

	offs = sz;
	sz = fabs(sz);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_push_matrix();

	for(i=0; i<6; i++) {
		m = i > 0 ? &tmpmesh : mesh;
		if(gen_plane_mesh(m, sz, sz, sub, sub) == -1)
			return -1;
		g3d_load_identity();
		g3d_rotate(rotface[i][0], rotface[i][1], rotface[i][2], rotface[i][3]);
		g3d_translate(0, 0, offs / 2.0f);
		apply_mesh_xform(m, g3d_get_matrix(G3D_MODELVIEW, 0));
		if(i > 0) {
			if(append_mesh(mesh, m) == -1) {
				return -1;
			}
		}
	}

	g3d_pop_matrix();
	return 0;
}

static void torusvec(float *res, float theta, float phi, float mr, float rr)
{
	float rx, ry, rz;
	theta = -theta;

	rx = -cos(phi) * rr + mr;
	ry = sin(phi) * rr;
	rz = 0.0f;

	res[0] = rx * sin(theta) + rz * cos(theta);
	res[1] = ry;
	res[2] = -rx * cos(theta) + rz * sin(theta);
}

int gen_torus_mesh(struct g3d_mesh *mesh, float rad, float ringrad, int usub, int vsub)
{
	int i, j;
	int nfaces, uverts, vverts;
	struct g3d_vertex *vptr;
	uint16_t *iptr;

	init_mesh(mesh, G3D_QUADS, 0, 0);

	if(usub < 4) usub = 4;
	if(vsub < 2) vsub = 2;

	uverts = usub + 1;
	vverts = vsub + 1;

	mesh->vcount = uverts * vverts;
	nfaces = usub * vsub;
	mesh->icount = nfaces * 4;

	printf("generating torus with %d faces (%d vertices)\n", nfaces, mesh->vcount);

	if(!(mesh->varr = malloc(mesh->vcount * sizeof *mesh->varr))) {
		return -1;
	}
	if(!(mesh->iarr = malloc(mesh->icount * sizeof *mesh->iarr))) {
		return -1;
	}
	vptr = mesh->varr;
	iptr = mesh->iarr;

	for(i=0; i<uverts; i++) {
		float u = (float)i / (float)(uverts - 1);
		float theta = u * 2.0 * M_PI;
		float rcent[3];

		torusvec(rcent, theta, 0, rad, 0);

		for(j=0; j<vverts; j++) {
			float v = (float)j / (float)(vverts - 1);
			float phi = v * 2.0 * M_PI;
			int chess = (i & 1) == (j & 1);

			torusvec(&vptr->x, theta, phi, rad, ringrad);
			vptr->w = 1.0f;

			vptr->nx = (vptr->x - rcent[0]) / ringrad;
			vptr->ny = (vptr->y - rcent[1]) / ringrad;
			vptr->nz = (vptr->z - rcent[2]) / ringrad;
			vptr->u = u;
			vptr->v = v;
			vptr->r = chess ? 255 : 64;
			vptr->g = 128;
			vptr->b = chess ? 64 : 255;
			++vptr;

			if(i < usub && j < vsub) {
				int idx = i * vverts + j;
				*iptr++ = idx;
				*iptr++ = idx + 1;
				*iptr++ = idx + vverts + 1;
				*iptr++ = idx + vverts;
			}
		}
	}
	return 0;
}

