#ifndef GFX_H_INCLUDED
#define GFX_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>

#define GFX_BG_COLOR 0xFF444444

typedef struct{
    int zoom_factor;
    int sw, sh; // scope width, scope height
    int s_origin_y; // y origin of the scope; 1052E origin is top left, 1054Z origin is bottom left ... FFS RIGOL!!!
    SDL_Rect spos;
    SDL_Surface *bg;

    int channels;
    int *enabled;
    SDL_Surface **chan;

    int ww, wh; //window width, window height
    SDL_Window *w;
    SDL_Surface *ws;

    TTF_Font *font;
}Gfx;

enum{
    GFX_CURSOR_POINT_UP = 0,
    GFX_CURSOR_POINT_DOWN,
    GFX_CURSOR_POINT_LEFT,
    GFX_CURSOR_POINT_RIGHT
};


Gfx* GFX_initWindow(int zoom_factor, int window_width, int window_height);
void GFX_initScope(Gfx* g, int x, int y, int width, int height, int origin_y, int channels);
void GFX_free(Gfx* g);
// Draw background
void GFX_generateBg(Gfx *g, int px_vert_div, int px_hori_div);
void GFX_drawChannel(Gfx *g, int channel, uint8_t *data, int max_data, uint32_t color);

void GFX_update(Gfx *g);
void GFX_drawScope(Gfx *g);
void GFX_clearScreen(Gfx *g);

void GFX_drawChannelCursor(Gfx *g, uint32_t color, float Voffset, float Vscale, uint32_t px_vert_div, uint32_t px_center);

void GFX_drawCursor(Gfx *g, uint32_t color, float offset, float scale, uint32_t px_div, uint32_t px_center, uint32_t cursor_dir);

void GFX_loadTTF(Gfx *g, char *path, int ptsize);
void GFX_printScopeInfo(Gfx *g, int x_offset, int y, float time_scale, float time_offset, uint32_t color);
void GFX_printChannelInfo(Gfx *g, int channel, int x_offset, int y, int y_offset, float Vscale, float Voffset, uint32_t color);

void GFX_enableChannel(Gfx *g, int channel);
void GFX_disableChannel(Gfx *g, int channel);

#endif // GFX_H_INCLUDED
