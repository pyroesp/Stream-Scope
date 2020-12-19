#include "DS1052E.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <visa.h> // http://www.ni.com/pdf/manuals/370132c.pdf


const uint32_t DS1052E_color[DS1052E_CHANNELS] = {DS1052E_COLOR_CH1, DS1052E_COLOR_CH2};


int DS1052E_initScope(void *pScope){
    int i;
    Scope *s = (Scope*)pScope;
    s->screen.zoom = DS1052E_ZOOM_FACTOR;
    s->screen.width = DS1052E_SCOPE_W;
    s->screen.height = DS1052E_SCOPE_H;
    s->screen.origin_x = DS1052E_ORIGIN_X;
    s->screen.origin_y = DS1052E_ORIGIN_Y;
    // pixels per division
    s->screen.px_vert_div = DS1052E_V_DIV;
    s->screen.px_hori_div = DS1052E_H_DIV;
    s->screen.px_center = DS1052E_BYTE_CENTER;

    s->wave.header = DS1052E_HEADER_SIZE;
    s->wave.max_data = DS1052E_MAX_WAVE_DATA;
    s->wave.max_data_run = DS1052E_MAX_DATA_RUN;
    s->wave.max_data_stop = DS1052E_MAX_DATA_STOP;

    s->channels = DS1052E_CHANNELS;

    s->c = (ScopeChannel*)malloc(sizeof(ScopeChannel) * s->channels);
    if (s->c == NULL){
        return -1;
    }

    /* Write to scope functions */
    s->fct.setLongMemory = &DS1052E_setLongMemory;
    // start / stop scope
    s->fct.stop = &DS1052E_stop;
    s->fct.run = &DS1052E_run;
    // Unlock scope
    s->fct.unlockScope = &DS1052E_unlockScope;
    // Enable / Disable channel
    s->fct.chanEnable = &DS1052E_chanEnable;
    s->fct.chanDisable = &DS1052E_chanDisable;

    /* Read from scope functions */
    // Scope time stuff
    s->fct.sampleRate = &DS1052E_sampleRate;
    s->fct.timeScale = &DS1052E_timeScale;
    s->fct.timeOffset = &DS1052E_timeOffset;
    // Channel specific functions
    s->fct.chanRead = &DS1052E_chanRead;
    s->fct.chanVScale = &DS1052E_chanVScale;
    s->fct.chanVOffset = &DS1052E_chanVOffset;
    s->fct.chanProbe = &DS1052E_chanProbe;
    s->fct.isChanEnabled = &DS1052E_isChanEnabled;

    /* Other functions */
    // Basic printf displaying all channel info
    s->fct.printChannelInfo = &DS1052E_printChannelInfo;

    for (i = 0; i < s->channels; i++){
        s->c[i].enabled = s->fct.isChanEnabled(pScope, i + 1);
        s->c[i].color = DS1052E_color[i];
        s->c[i].rd_buf = (uint8_t*)malloc(sizeof(uint8_t) * s->wave.max_data);
        if (s->c[i].rd_buf == NULL)
            return -1;
        s->c[i].raw_data = (uint8_t*)malloc(sizeof(uint8_t) * s->wave.max_data_stop);
        if (s->c[i].raw_data == NULL)
            return -1;
    }

    return 0;
}

int DS1052E_chanRead(void *pScope, int channel){
    uint32_t ret_cnt;
    ViStatus vi_status;
    char tmc_size;
    char read_size_data_arg[16] = "%*c%*c%0d";
    Scope *s = (Scope*)pScope;
    ScopeChannel *c = &s->c[channel-1];
    viPrintf(s->vi, ":WAV:DATA? CHAN%d\n", channel); // Ask for channel data
    vi_status = viRead(s->vi, c->rd_buf, s->wave.max_data, (ViPUInt32)&ret_cnt); // Read 1024 bytes if available
    if (vi_status != VI_SUCCESS && vi_status != VI_SUCCESS_MAX_CNT){
        return -1;
    }

    // read length from header (8 for DS1052E / 9 for  DS1054Z)
    sscanf((char*)c->rd_buf, "%*c%c", &tmc_size);
    strchr(read_size_data_arg, '0')[0] = tmc_size;
    sscanf((char*)c->rd_buf, read_size_data_arg, &c->size_raw_data);

    if (ret_cnt >= c->size_raw_data){
        memcpy(c->raw_data, &c->rd_buf[s->wave.header], sizeof(uint8_t) * s->wave.max_data_run);
    }else{
        /**
            DS1052E : From tests, in stop mode with long mem depth, the first 1024 include the 10 bytes header
                      The 10 bytes header is included into the total 1M or 512K samples
        **/
        memcpy(&c->raw_data[0], &c->rd_buf[s->wave.header], sizeof(uint8_t) * (s->wave.max_data - s->wave.header));
        uint32_t raw_data_idx = s->wave.max_data - s->wave.header;
        for (; raw_data_idx < c->size_raw_data - s->wave.header;){
            /** Read directly into raw_data array. After first read, everything is data **/
            vi_status = viRead(s->vi, &c->raw_data[raw_data_idx], s->wave.max_data, (ViPUInt32)&ret_cnt); // Read X data bytes
            if (vi_status != VI_SUCCESS && vi_status != VI_SUCCESS_MAX_CNT)
                return -1;
            raw_data_idx += ret_cnt;
        }
    }
    return 0;
}

void DS1052E_run(void *pScope){
    Scope *s = (Scope*)pScope;
    // Run the scope
    viPrintf(s->vi, ":RUN\n");
    s->running = 1;
}

void DS1052E_stop(void *pScope){
    Scope *s = (Scope*)pScope;
    // Stop the scope
    viPrintf(s->vi, ":STOP\n");
    s->running = 0;
}

void DS1052E_setLongMemory(void *pScope){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":ACQ:MEMD LONG\n"); // Set Memdepth to Long
    viPrintf(s->vi, ":WAV:POIN:MODE MAX\n"); // Set Waveform Point Mode to MAX
}

void DS1052E_unlockScope(void *pScope){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":KEY:LOCK DIS\n"); // Unlock oscilloscope keys
}

void DS1052E_sampleRate(void *pScope){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":ACQ:SAMP?\n"); // Ask Sample rate
    viScanf(s->vi, "%t\n", &buf[0]);
    s->sample_rate = strtof(&buf[0], &p);
}

void DS1052E_timeScale(void *pScope){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":TIM:SCAL?\n");
    viScanf(s->vi, "%t\n", buf);
    s->time_scale = strtof(&buf[0], &p);
}

void DS1052E_timeOffset(void *pScope){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":TIM:OFFS?\n");
    viScanf(s->vi, "%t\n", buf);
    s->time_offset = strtof(&buf[0], &p);
}

void DS1052E_chanVScale(void *pScope, int channel){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:SCAL?\n", channel); // Ask Chan V Scale
    viScanf(s->vi, "%t\n", &buf[0]);
    s->c[channel-1].Vscale = strtof(&buf[0], &p);
}

void DS1052E_chanVOffset(void *pScope, int channel){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:OFFS?\n", channel); // Ask Chan V Offset
    viScanf(s->vi, "%t\n", &buf[0]);
    s->c[channel-1].Voffset = strtof(&buf[0], &p);
}

void DS1052E_chanProbe(void *pScope, int channel){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:PROB?\n", channel); // Ask Chan probe
    viScanf(s->vi, "%t\n", &buf[0]);
    s->c[channel-1].probe = strtof(&buf[0], &p);
}

void DS1052E_chanEnable(void *pScope, int channel){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:DISP ON\n", channel);
    s->c[channel-1].enabled = 1;
}

void DS1052E_chanDisable(void *pScope, int channel){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:DISP OFF\n", channel);
    s->c[channel-1].enabled = 0;
}

int DS1052E_isChanEnabled(void *pScope, int channel){
    int enabled;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:DISP?\n", channel);
    viScanf(s->vi, "%t\n", buf);
    sscanf(buf, "%d", &enabled);
    s->c[channel - 1].enabled = enabled;
    return enabled;
}

void DS1052E_printChannelInfo(void *pScope, int channel){
    Scope *s = (Scope*)pScope;
    printf("Sample rate : %fHz\n", s->sample_rate);
    printf("Time scale: %fs\n", s->time_scale);
    printf("Time offset: %fs\n", s->time_offset);

    printf("Channel Vscale : %fV\n", s->c[channel-1].Vscale);
    printf("Channel Voffset : %fV\n", s->c[channel-1].Voffset);
    printf("Channel probe : x%f\n", s->c[channel-1].probe);
}
