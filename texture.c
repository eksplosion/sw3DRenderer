#include "texture.h"

void texture_load(const char *filename, texture *t)
{
    FILE *f = fopen(filename, "r");

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, f);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    t->width = png_get_image_width(png_ptr, info_ptr);
    t->height = png_get_image_height(png_ptr, info_ptr);

    t->point = malloc(sizeof(Uint32) * t->width * t->height);
    for(int y = 0; y < t->height; y++)
        for(int x = 0; x < t->width; x++)
        {
            Uint8 r, g, b;
            r = ((Uint32 *)row_pointers[y])[x] >> 16;
            g = ((Uint32 *)row_pointers[y])[x] >> 8;
            b = ((Uint32 *)row_pointers[y])[x];
            t->point[y * t->width + x] = (b << 16) + (g << 8) + r;
        }

    printf("Loaded texture from \"%s\" with %dx%d pixels!\n", filename, t->width, t->height);

    png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)NULL);
    fclose(f);
}

Uint32 texture_sample(texture *t, float u, float v)
{
    int x = u * (t->width-1);
    int y = (1.f - v) * (t->height-1);

    return t->point[y * t->width + x];
}

void texture_group_create(texture_group *tg)
{
    tg->t = NULL;
    tg->size = 0;
}

void texture_group_texture_add(texture_group *tg, const char *filename)
{
    tg->size++;
    tg->t = realloc(tg->t, sizeof(texture) * tg->size);
    texture_load(filename, &tg->t[tg->size-1]);
}
