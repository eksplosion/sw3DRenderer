#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL_types.h>

#define DEG_TO_RAD 0.01745329252f

typedef struct{
	float u, v;
} vec2;

typedef struct{
	float x, y, z;
} vec3;

typedef struct{
	float x, y, z, w;
} vec4;

typedef struct{
	vec3 vert[3];
	vec3 cache[3];
	vec2 tex[3];
    Uint32 color;
	int tex_index;
} triangle;

typedef struct{
    triangle *t;
    Uint32 size;
    Uint32 allocated;
    vec3 pos;
	vec3 rotation;
	int processed;
} mesh;

typedef struct{
    float m[4][4];
} matrix;

typedef struct{
	int width;
	int height;
	Uint32 *point;
} texture;

typedef struct{
	texture *t;
	Uint32 size;
} texture_group;

#endif // TYPES_H
