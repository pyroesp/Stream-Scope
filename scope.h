#ifndef SCOPE_H_INCLUDED
#define SCOPE_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
// check whether __int64 is defined, if not then define it
// add your own compiler definition here, I'm just lazy
// PS: needed for visa
#ifndef __int64
    #ifdef __MINGW32__
        #include <_mingw.h>
    #else
        #define __int64 long long
    #endif // __int64
#endif

#include <visa.h> // http://www.ni.com/pdf/manuals/370132c.pdf

#define SCOPE_ID_BUFF_SIZE 100

typedef struct _ScopeChannel{
    float Vscale; // voltage scale
    float Voffset; // voltage offset
    float probe; // 1x, 10x, ...
    uint8_t *rd_buf; // read buffer for VISA data
    uint32_t size_raw_data; // size of channel raw data / to read (comes from TMC header)
    uint8_t *raw_data; // channel raw data

    uint32_t color; // channel color

    uint8_t enabled; // channel enabled ?
}ScopeChannel;

typedef struct _ScopeScreen{
    // Zoom factor for gfx lib
    uint32_t zoom;
    // Scope width and height
    uint32_t width, height;
    // Origin of scope screen
    uint32_t origin_x, origin_y;
    // Pixels per division
    uint32_t px_vert_div;
    uint32_t px_hori_div;
    // Value of pixel at center of scope screen
    uint32_t px_center;
}ScopeScreen;

typedef struct _ScopeWave{
    // Waveform command information (sizes)
    int header; // header size
    int max_data; // max data per transfer
    int max_data_stop; // total data to read when scope is stopped (1 channel active)
    int max_data_run; // total data to read when scope is running
}ScopeWave;

typedef struct _ScopeFunctions{
    int (*initScope)(void *pScope); // scope initialization function

    /* Write to scope functions */
    void (*setLongMemory)(void *pScope);
    // start / stop scope
    void (*stop)(void *pScope);
    void (*run)(void *pScope);
    // Unlock scope
    void (*unlockScope)(void *pScope);
    // Enable / Disable channel
    void (*chanEnable)(void*, int);
    void (*chanDisable)(void*, int);

    /* Read from scope functions */
    // Scope time stuff
    void (*sampleRate)(void *pScope);
    void (*timeScale)(void *pScope);
    void (*timeOffset)(void *pScope);
    // Channel specific functions
    int (*chanRead)(void*, int);
    void (*chanVScale)(void*, int);
    void (*chanVOffset)(void*, int);
    void (*chanProbe)(void*, int);
    int (*isChanEnabled)(void*, int);

    /* Other functions */
    // Basic printf displaying all channel info
    void (*printChannelInfo)(void*, int);
}ScopeFunctions;

typedef struct _Scope{
    // VISA connection stuff
    ViSession defaultRM, vi;

    // scope device ID
    char deviceID[SCOPE_ID_BUFF_SIZE];
    // Scope screen information
    ScopeScreen screen;
    // Waveform command information (sizes)
    ScopeWave wave;

    // available channels
    uint8_t channels;
    // channel data array
    ScopeChannel *c;

    // sample rate
    float sample_rate;
    float time_scale; // time scale
    float time_offset; // trigger offset

    // running (1), or stopped (0)
    uint8_t running;

    // scope read/write functions
    ScopeFunctions fct;
}Scope;


int SCOPE_openVISA(Scope *pScope);
void SCOPE_getID(Scope *pScope);
void SCOPE_closeVISA(Scope *pScope);

void SCOPE_freeScope(Scope *pScope);


#endif // SCOPE_H_INCLUDED
