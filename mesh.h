#ifndef MESH_H
#define MESH_H

#include "types.h"
#include "global.h"

enum{
    X, Y, Z
};

void mesh_create(mesh *m, Uint32 size);
void mesh_update(mesh *m, Uint32 size);
void mesh_triangle_add(mesh *m, triangle t);
int mesh_load(const char *filename, mesh *m, Uint32 color, int tex_index);
void mesh_process(mesh *m);
void vec2_swap(vec2 *v1, vec2 *v2);
void vec3_swap(vec3 *v1, vec3 *v2);
void intersect(vec3 p1, vec3 p2, vec2 t1, vec2 t2, vec3 l, vec3 plane, vec3 *o, vec2 *ot);
int triangle_clip(triangle *t, int clipped, triangle *t_out1, triangle *t_out2);
void triangle_rotate(triangle *in, triangle *out, float matrix[3][3]);
vec3 triangle_normal(triangle *t);

#endif // MESH_H
