#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "gfx.h"
#include "scope.h"
#include "DS1052E.h"

// DS1054Z is commented because of very slow FPS when all channels are active
// Also because RIGOL changed the commands on how to read channel data between the DS1052E and the DS1054Z
// and the way they're doing it on the DS1054Z is fucking stupid
// I leave it here if someone wants to mess around with it, but there are some graphic bugs like
// the waveform and the cursor not displaying at the correct height on the scope,
// because I initially made this for the DS1052E. I've probably used some magic numbers or
// done some calculations that work on the DS1052E but not on the DS1054Z
// #include "DS1054Z.h"

#define BORDER 10
#define FONT_SIZE 8
#define TEXT_INFO_Y (FONT_SIZE + 2)
#define FONT "LiberationMono-Regular.ttf"

int main(int argc, char* argv[]){
    int i, exit = 0;

    Scope *scope;
    scope = (Scope*)malloc(sizeof(Scope));

    if (!scope){
        printf("Scope returned NULL!\n");
        return -1;
    }

    if (SCOPE_openVISA(scope) != 0){
        printf("Error opening device.\nCheck if device is plugged in and powered on.\n");
        return -1;
    }
    SCOPE_getID(scope);
    if (strstr(scope->deviceID, "DS1052E")){
        scope->fct.initScope = &DS1052E_initScope;
#ifdef DS1054Z_H_INCLUDED
    }else if (strstr(scope->deviceID, "DS1054Z")){
        scope->fct.initScope = &DS1054Z_initScope;
#endif
    }else{
        SCOPE_closeVISA(scope);
        printf("Device not supported.\nDevice: %s.\n", scope->deviceID);
        return -1; // device not supported
    }

    if (scope->fct.initScope(scope) != 0){
        printf("Error during initialization");
        return -1;
    }

    scope->fct.setLongMemory(scope);
    scope->fct.sampleRate(scope);
    scope->fct.timeScale(scope);
    scope->fct.timeOffset(scope);
    scope->fct.chanVScale(scope, 1);
    scope->fct.chanVOffset(scope, 1);
    scope->fct.chanProbe(scope, 1);
    scope->fct.run(scope);
    scope->fct.printChannelInfo(scope, 1);

    Gfx *gfx;
    gfx = GFX_initWindow(scope->screen.zoom, scope->screen.width * scope->screen.zoom + BORDER, (scope->screen.height + TEXT_INFO_Y * scope->channels) * scope->screen.zoom + BORDER);
    if (gfx == NULL){
        printf("GFX init error\n");
        return -1;
    }
    GFX_initScope(gfx, BORDER/2, BORDER/2, scope->screen.width, scope->screen.height, scope->screen.origin_y, scope->channels);
    GFX_loadTTF(gfx, FONT, FONT_SIZE * scope->screen.zoom);
    if (gfx->font == NULL){
        printf("Error loading font.\n");
        return -1;
    }

    GFX_clearScreen(gfx); // clear screen for text at the bottom
    GFX_printScopeInfo(gfx, BORDER / 2, scope->screen.height * scope->screen.zoom + BORDER, scope->time_scale, scope->time_offset, 0xFFFFFFFF); // print scope information bottom right
    for (i= 0; i < scope->channels; i++){
        if (scope->fct.isChanEnabled(scope, i + 1)){
            GFX_drawChannelCursor(gfx, scope->c[i].color, scope->c[i].Voffset, scope->c[i].Vscale, scope->screen.px_vert_div, scope->screen.px_center);
            GFX_printChannelInfo(gfx, i + 1, BORDER / 2, scope->screen.height * scope->screen.zoom + BORDER, 2, scope->c[i].Vscale, scope->c[i].Voffset, scope->c[i].color); // print channel info bottom left
        }
    }
    GFX_generateBg(gfx, scope->screen.px_vert_div, scope->screen.px_hori_div);

    for (i = 0; i < scope->channels; i++){
        if (scope->fct.isChanEnabled(scope, i + 1)){
            if (scope->fct.chanRead(scope, i + 1) == 0)
                GFX_drawChannel(gfx, i + 1, scope->c[i].raw_data, scope->wave.max_data_run, scope->c[i].color);
            GFX_enableChannel(gfx, i + 1);
        }else{
            GFX_disableChannel(gfx, i + 1);
        }
    }
    GFX_drawScope(gfx);
    GFX_update(gfx);

    SDL_Event e;

    uint32_t locked = 1, read_channel_info = 1;
    while(!exit){
        while(SDL_PollEvent(&e)){
            switch(e.type){
                case SDL_QUIT:
                    exit = 1;
                    break;
                case SDL_KEYDOWN:
                    switch(e.key.keysym.sym){
                        case SDLK_ESCAPE:
                            exit = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch(e.key.keysym.sym){
                        case SDLK_l: // LOCK / UNLOCK scope with L key
                            if (locked){
                                scope->fct.unlockScope(scope);
                                locked = 0;
                                read_channel_info = 1;
                            }else{
                                locked = 1;
                                if (read_channel_info){
                                    GFX_clearScreen(gfx); // clear screen for text at the bottom

                                    scope->fct.sampleRate(scope);
                                    scope->fct.timeScale(scope);
                                    scope->fct.timeOffset(scope);

                                    // TODO draw trigger cursor


                                    GFX_printScopeInfo(gfx, BORDER / 2, scope->screen.height * scope->screen.zoom + BORDER, scope->time_scale, scope->time_offset, 0xFFFFFFFF); // print scope information bottom right
                                    for (i= 0; i < scope->channels; i++){
                                        if (scope->fct.isChanEnabled(scope, i + 1)){
                                            // Get channel info
                                            scope->fct.chanVScale(scope, i + 1);
                                            scope->fct.chanVOffset(scope, i + 1);
                                            scope->fct.chanProbe(scope, i + 1);
                                            // Draw channel info
                                            GFX_enableChannel(gfx, i + 1);
                                            GFX_drawChannelCursor(gfx, scope->c[i].color, scope->c[i].Voffset, scope->c[i].Vscale, scope->screen.px_vert_div, scope->screen.px_center);
                                            GFX_printChannelInfo(gfx, i + 1, BORDER / 2, scope->screen.height * scope->screen.zoom + BORDER, 2, scope->c[i].Vscale, scope->c[i].Voffset, scope->c[i].color); // print channel info bottom left
                                        }else{
                                            GFX_disableChannel(gfx, i + 1);
                                        }
                                    }

                                    read_channel_info = 0;
                                }
                            }
                            break;
                    }
                    break;
            }
        }

        if (locked){
            for (i = 0; i < scope->channels; i++){
                if (scope->c[i].enabled){
                    if (scope->fct.chanRead(scope, i + 1) == 0){
                        GFX_drawChannel(gfx, i + 1, scope->c[i].raw_data, scope->wave.max_data_run, scope->c[i].color);
                    }
                }
            }
            GFX_drawScope(gfx);
            GFX_update(gfx);
        }
    }

    //Disable and destroy Gfx stuff
    GFX_free(gfx);

    // Disable and destroy scope stuff
    scope->fct.unlockScope(scope);
    SCOPE_closeVISA(scope);
    SCOPE_freeScope(scope);
    free(scope);

    return 0;
}
