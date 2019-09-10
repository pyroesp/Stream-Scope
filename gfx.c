#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gfx.h"

Gfx* GFX_initWindow(int zoom_factor, int window_width, int window_height){
    Gfx *g = (Gfx*)malloc(sizeof(Gfx));
    if (!g)
        return NULL;
    if (SDL_Init(SDL_INIT_VIDEO)){
        printf("SDL could not initialize... SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }
    if (TTF_Init()){
        printf("TTF could not initialize... TTF_Error: %s\n", TTF_GetError());
        return NULL;
    }
    g->zoom_factor = zoom_factor;
    g->ww = window_width;
    g->wh = window_height;
    g->w = SDL_CreateWindow("Stream Scope", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         window_width, window_height, SDL_WINDOW_SHOWN);
    g->ws = SDL_GetWindowSurface(g->w);

    return g;
}

void GFX_initScope(Gfx* g, int x, int y, int width, int height, int origin_y, int channels){
    int i;
    g->sw = width;
    g->sh = height;
    g->s_origin_y = origin_y;
    g->spos.x = x;
    g->spos.y = y;
    g->spos.w = 0;
    g->spos.h = 0;

    g->bg = SDL_CreateRGBSurface(0, g->sw*g->zoom_factor, g->sh*g->zoom_factor, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    g->channels = channels;
    g->chan = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * channels);
    g->enabled = (int*)malloc(sizeof(int) * channels);
    for (i = 0; i < channels; i++){
        g->enabled[i] = 0;
        g->chan[i] = SDL_CreateRGBSurface(0, g->sw*g->zoom_factor, g->sh*g->zoom_factor, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        SDL_FillRect(g->chan[i], NULL, 0x00FFFFFF);
    }
}

void GFX_free(Gfx* g){
    SDL_FreeSurface(g->bg);
    SDL_FreeSurface(g->ws);
    TTF_CloseFont(g->font);
    SDL_DestroyWindow(g->w);
    SDL_Quit();
    free(g->chan);
    free(g->enabled);
    free(g);
}

void GFX_update(Gfx *g){
    SDL_UpdateWindowSurface(g->w);
}


void GFX_clearScreen(Gfx *g){
    SDL_FillRect(g->ws, NULL, GFX_BG_COLOR);
}

void GFX_drawScope(Gfx *g){
    int i;
    SDL_BlitSurface(g->bg, NULL, g->ws, &g->spos);
    for (i = 0; i < g->channels; i++)
        if (g->enabled[i])
            SDL_BlitSurface(g->chan[i], NULL, g->ws, &g->spos);
}

void GFX_enableChannel(Gfx *g, int channel){
    g->enabled[channel-1] = 1;
}

void GFX_disableChannel(Gfx *g, int channel){
    g->enabled[channel-1] = 0;
}

SDL_Rect GFX_setRect(int x, int y, int w, int h){
    SDL_Rect r = {x, y, w, h};
    return r;
}

void GFX_setPixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    uint32_t *target_pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * sizeof(*target_pixel));
    *target_pixel = pixel;
}

// Draw background onto bg surface
void GFX_generateBg(Gfx *g, int px_vert_div, int px_hori_div){
    int x, y;
    SDL_FillRect(g->bg, NULL, 0xFF222222);
    for (y = 0; y < g->sh*g->zoom_factor; y++){
        for (x = 0; x < g->sw*g->zoom_factor; x++){
            /** Draw edges **/
            if (x == 0 || y == 0){
                GFX_setPixel(g->bg, x, y, 0xFFAAAAAA);
                if (y % (5*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x+1, y, 0xFFAAAAAA);
                if (x % (5*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x, y+1, 0xFFAAAAAA);
                if (y % (px_vert_div*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x+2, y, 0xFFAAAAAA);
                if (x % (px_hori_div*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x, y+2, 0xFFAAAAAA);
            }
            if (x == g->sw*g->zoom_factor-1 || y == g->sh*g->zoom_factor-1){
                GFX_setPixel(g->bg, x, y, 0xFFAAAAAA);
                if (y % (5*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x-1, y, 0xFFAAAAAA);
                if (x % (5*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x, y-1, 0xFFAAAAAA);
                if (y % (px_vert_div*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x-2, y, 0xFFAAAAAA);
                if (x % (px_hori_div*g->zoom_factor) == 0)
                    GFX_setPixel(g->bg, x, y-2, 0xFFAAAAAA);
            }

            /** Draw internal dots **/
            if ((x % (px_hori_div*g->zoom_factor)) == 0)
                if ((y % (5*g->zoom_factor)) == 0)
                    GFX_setPixel(g->bg, x, y, 0xFFAAAAAA);
            if ((y % (px_vert_div*g->zoom_factor)) == 0)
                if ((x % (5*g->zoom_factor)) == 0)
                    GFX_setPixel(g->bg, x, y, 0xFFAAAAAA);

            /** Draw Vertical **/
            if (x >= ((g->sw/2) - 1)*g->zoom_factor && x <= ((g->sw/2) + 1)*g->zoom_factor)
                if ((y % (5*g->zoom_factor)) == 0)
                    GFX_setPixel(g->bg, x, y, 0xFFAAAAAA);

            /**  Draw Horizontal **/
            if (y >= ((g->sh/2) - 1)*g->zoom_factor && y <= ((g->sh/2) + 1)*g->zoom_factor)
                if ((x % (5*g->zoom_factor)) == 0)
                    GFX_setPixel(g->bg, x, y, 0xFFAAAAAA);
        }
    }
}

void GFX_drawLine(SDL_Surface *s, uint32_t c, int x1, int y1, int x2, int y2){
    float dx, x;
    float dy, y;
    float step, i;

    dx = (x2 - x1);
    dy = (y2 - y1);
    if(fabs(dx) >= fabs(dy))
        step = fabs(dx);
    else
        step = fabs(dy);
    dx = dx / step;
    dy = dy / step;
    x = x1;
    y = y1;
    i = 0;
    while(i <= step) {
        GFX_setPixel(s, (int)x, (int)y, c);
        x = x + dx;
        y = y + dy;
        i = i + 1;
    }
}

void GFX_drawChannel(Gfx *g, int channel, uint8_t *data, int max_data, uint32_t color){
    float x;
    float x_offset = (float)g->sw / (float)max_data;
    uint32_t idx;
    SDL_FillRect(g->chan[channel-1], NULL, 0x00FFFFFF);
    if (g->enabled[channel - 1]){
        for (x = 0.0, idx = 0; (int)x < g->sw && (idx+1) < max_data; x += x_offset, idx++){
            GFX_drawLine(g->chan[channel-1], color, x * g->zoom_factor,
                         abs(g->s_origin_y - data[idx]) * g->zoom_factor, (x+x_offset) * g->zoom_factor, //x1, y1
                         abs(g->s_origin_y - data[idx+1]) * g->zoom_factor);                            // x2, y2
        }
    }
}

void GFX_drawChannelCursor(Gfx *g, uint32_t color, float Voffset, float Vscale, uint32_t px_vert_div, uint32_t px_center){
    float V_per_px = Vscale / (float)px_vert_div;
    uint32_t y;

    if (Voffset < 0)
        y = px_center + (uint32_t)floor(fabs(Voffset) / V_per_px);
    else
        y = px_center - (uint32_t)floor(Voffset / V_per_px);

    SDL_Surface *cursor = SDL_CreateRGBSurface(0, 5, 5, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_FillRect(cursor, NULL, 0x00FFFFFF);
    SDL_Rect r = {0,0,3,5};
    SDL_FillRect(cursor, &r, color);
    r.x = 3; r.y = 1; r.w = 1; r.h = 3;
    SDL_FillRect(cursor, &r, color);
    r.x = 4; r.y = 3; r.w = 1; r.h = 1;
    SDL_FillRect(cursor, &r, color);

    r.x = 0; r.y = y * g->zoom_factor + 3; r.w = 0; r.h = 0;
    SDL_BlitSurface(cursor, NULL, g->ws, &r);

    SDL_FreeSurface(cursor);
}

void GFX_loadTTF(Gfx *g, char *path, int ptsize){
    g->font = TTF_OpenFont(path, ptsize);
}

void GFX_toEngNotation(double d, int digits, int precision, char *p){
    char param[16];
    double exponent = log10(fabs(d));
    sprintf(param, "%%%d.%df", digits, precision);
    if (fabs(d) >= 1){
        switch ((int)floor(exponent))
        {
            case 0: case 1: case 2:
                sprintf(p, param, d);
                break;
            case 3: case 4: case 5:
                strcat(param, "k");
                sprintf(p, param, d/1e3);
                break;
            case 6: case 7: case 8:
                strcat(param, "M");
                sprintf(p, param, d/1e6);
                break;
            case 9: case 10: case 11:
                strcat(param, "G");
                sprintf(p, param, d/1e9);
                break;
            default:
                strcat(param, "T");
                sprintf(p, param, d/1e12);
                break;
        }
    }else if (fabs(d) > 0){
        switch ((int)floor(exponent))
        {
            case -1: case -2: case -3:
                strcat(param, "m");
                sprintf(p, param, d*1e3);
                break;
            case -4: case -5: case -6:
                strcat(param, "u");
                sprintf(p, param, d*1e6);
                break;
            case -7: case -8: case -9:
                strcat(param, "n");
                sprintf(p, param, d*1e9);
                break;
            case -10: case -11: case -12:
                strcat(param, "p");
                sprintf(p, param, d*1e12);
                break;
            default:
                strcat(param, "f");
                sprintf(p, param, d*1e15);
                break;
        }
    }else{
        strcat(param, " ");
        sprintf(p, param, 0.0);
    }
}

void GFX_printScopeInfo(Gfx *g, int x_offset, int y, float time_scale, float time_offset, uint32_t color){
    char scale[16], offset[16];
    char buff[32];
    SDL_Surface *text;
    GFX_toEngNotation((double)time_scale, 5, 1, scale);
    GFX_toEngNotation((double)time_offset, 5, 1, offset);
    sprintf(buff, "Time: %ss | Offs: %ss", scale, offset);
    SDL_Color c = {color& 0xFF , (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF};
    text = TTF_RenderText_Solid(g->font, buff, c);
    if (text == NULL){
        printf("Scope info: Error in rendering text\n");
        return;
    }
    SDL_Rect r = {g->ww - text->w - x_offset, y, 0, 0};
    SDL_BlitSurface(text, NULL, g->ws, &r);
    SDL_FreeSurface(text);
}

void GFX_printChannelInfo(Gfx *g, int channel, int x_offset, int y, int y_offset, float Vscale, float Voffset, uint32_t color){
    char scale[16], offset[16];
    char buff[32];
    SDL_Surface *text;
    GFX_toEngNotation((double)Vscale, 4, 2, scale);
    GFX_toEngNotation((double)Voffset, 4, 2, offset);
    sprintf(buff, "CH%d: %sV | %sV", channel, scale, offset);
    SDL_Color c = {color& 0xFF , (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF};
    text = TTF_RenderText_Solid(g->font, buff,  c);
    if (text == NULL){
        printf("Channel info: Error in rendering text\n");
        return;
    }
    SDL_Rect r = {x_offset, y + y_offset + text->h * (channel - 1), 0, 0};
    SDL_BlitSurface(text, NULL, g->ws, &r);
    SDL_FreeSurface(text);
}
