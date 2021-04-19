#ifndef _GFX_H
#define _GFX_H

#define GFX_MAX_TEXTURES                (5)

int gfx_generate_bitmap(const char* fname);
void gfx_free_bitmap(int index);
int gfx_get_bitmap_width(int index);
int gfx_get_bitmap_height(int index);
void gfx_get_bitmap_color(int index, int x, int y, int* r, int* g, int* b);
void gfx_putpixel(int x, int y, int r, int g, int b);

#endif