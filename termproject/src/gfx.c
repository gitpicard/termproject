#include <stdlib.h>
#include <SDL.h>
#include "gfx.h"

SDL_Renderer* gfx_renderer = NULL;
SDL_Surface* gfx_textures[GFX_MAX_TEXTURES] = { NULL };

int gfx_generate_bitmap(const char* fname) {
    int i;
    /* Find a open texture slot that we can use if there is one. */
    for (i = 0; i < GFX_MAX_TEXTURES; i++) {
        if (gfx_textures[i] == NULL) {
            gfx_textures[i] = SDL_LoadBMP(fname);
            return i;
        }
    }
    /* There is no free texture space. */
    return -1;
}

void gfx_free_bitmap(int index) {
    if (index < GFX_MAX_TEXTURES && index >= 0) {
        if (gfx_textures[index] != NULL) {
            SDL_FreeSurface(gfx_textures[index]);
        }
    }
}

int gfx_get_bitmap_width(int index) {
    if (index < GFX_MAX_TEXTURES && index >= 0) {
        if (gfx_textures[index] != NULL) {
            return gfx_textures[index]->w;
        }
    }
    /* There is no texture with this index. */
    return -1;
}

int gfx_get_bitmap_height(int index) {
    if (index < GFX_MAX_TEXTURES && index >= 0) {
        if (gfx_textures[index] != NULL) {
            return gfx_textures[index]->h;
        }
    }
    /* There is no texture with this index. */
    return -1;
}

void gfx_get_bitmap_color(int index, int x, int y, int* r, int* g, int* b) {
    SDL_Surface* surf;
    int bpp;
    Uint8* pixel;
    Uint32 color;
    Uint8 red, green, blue;

    if (index < GFX_MAX_TEXTURES && index >= 0) {
        if (gfx_textures[index] != NULL) {
            surf = gfx_textures[index];
            bpp = surf->format->BytesPerPixel;

            /* Make sure we are not out of range. */
            if (x < 0 || y < 0 || x >= surf->w || y >= surf->h)
                return;

            pixel = (Uint8*)surf->pixels + (y * surf->pitch) + (x * bpp);
            color = *(Uint32*)pixel;

            /* Convert from the surface pixel format to the raw byte values. */
            SDL_GetRGB(color, surf->format, &red, &green, &blue);
            /* Now when we have everything we can set the outputs. */
            *r = red;
            *g = green;
            *b = blue;
        }
    }
    /* There is no texture with this index so do nothing. */
}

void gfx_putpixel(int x, int y, int r, int g, int b) {
    if (gfx_renderer != NULL) {
        SDL_SetRenderDrawColor(gfx_renderer, r, g, b, 255);
        SDL_RenderDrawPoint(gfx_renderer, x, y);
    }
}