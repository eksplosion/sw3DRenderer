#ifndef TEXTURE_H
#define TEXTURE_H

#include <png.h>
#include "types.h"

void texture_load(const char *filename, texture *t);
Uint32 texture_sample(texture *t, float u, float v);
void texture_group_texture_add(texture_group *tg, const char *filename);

#endif // TEXTURE_H
