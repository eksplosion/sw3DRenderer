#ifndef RENDER_H
#define RENDER_H

#include "mesh.h"
#include "global.h"
#include "texture.h"
#include "immintrin.h"

void matrix_multiply(vec4 *i, vec4 *o, matrix *mat);
void line_draw_default(int x1, int x2, float z1, float z2, int y, Uint32 color);
void line_draw_textured(int x1, int x2, float u1, float u2, float v1, float v2, float z1, float z2, int y, int tex_index);
void triangle_rasterize(triangle *tr);
void triangle_render(triangle *t);
void mesh_render(mesh *m);

#endif // RENDER_H
