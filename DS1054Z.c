#include "DS1054Z.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <visa.h> // http://www.ni.com/pdf/manuals/370132c.pdf


const uint32_t DS1054Z_color[DS1054Z_CHANNELS] = {DS1054Z_COLOR_CH1, DS1054Z_COLOR_CH2, DS1054Z_COLOR_CH3, DS1054Z_COLOR_CH4};


int DS1054Z_initScope(void *pScope){
    int i;
    Scope *s = (Scope*)pScope;
    s->screen.zoom = DS1054Z_ZOOM_FACTOR;
    s->screen.width = DS1054Z_SCOPE_W;
    s->screen.height = DS1054Z_SCOPE_H;
    s->screen.origin_x = DS1054Z_ORIGIN_X;
    s->screen.origin_y = DS1054Z_ORIGIN_Y;
    // pixels per division
    s->screen.px_vert_div = DS1054Z_V_DIV;
    s->screen.px_hori_div = DS1054Z_H_DIV;
    s->screen.px_center = DS1054Z_BYTE_CENTER;

    s->wave.header = DS1054Z_HEADER_SIZE;
    s->wave.max_data = DS1054Z_MAX_WAVE_DATA;
    s->wave.max_data_run = DS1054Z_MAX_DATA_RUN;
    s->wave.max_data_stop = DS1054Z_MAX_DATA_STOP;

    s->channels = DS1054Z_CHANNELS;

    s->c = (ScopeChannel*)malloc(sizeof(ScopeChannel) * s->channels);
    if (s->c == NULL){
        return -1;
    }

    /* Write to scope functions */
    s->fct.setLongMemory = &DS1054Z_setLongMemory;
    // start / stop scope
    s->fct.stop = &DS1054Z_stop;
    s->fct.run = &DS1054Z_run;
    // Unlock scope
    s->fct.unlockScope = &DS1054Z_unlockScope;
    // Enable / Disable channel
    s->fct.chanEnable = &DS1054Z_chanEnable;
    s->fct.chanDisable = &DS1054Z_chanDisable;

    /* Read from scope functions */
    // Scope time stuff
    s->fct.sampleRate = &DS1054Z_sampleRate;
    s->fct.timeScale = &DS1054Z_timeScale;
    s->fct.timeOffset = &DS1054Z_timeOffset;
    // Channel specific functions
    s->fct.chanRead = &DS1054Z_chanRead;
    s->fct.chanVScale = &DS1054Z_chanVScale;
    s->fct.chanVOffset = &DS1054Z_chanVOffset;
    s->fct.chanProbe = &DS1054Z_chanProbe;
    s->fct.isChanEnabled = &DS1054Z_isChanEnabled;

    /* Other functions */
    // Basic printf displaying all channel info
    s->fct.printChannelInfo = &DS1054Z_printChannelInfo;

    for (i = 0; i < s->channels; i++){
        s->c[i].enabled = s->fct.isChanEnabled(pScope, i + 1);
        s->c[i].color = DS1054Z_color[i];
        s->c[i].rd_buf = (uint8_t*)malloc(sizeof(uint8_t) * s->wave.max_data);
        if (s->c[i].rd_buf == NULL)
            return -1;
        s->c[i].raw_data = (uint8_t*)malloc(sizeof(uint8_t) * s->wave.max_data_stop);
        if (s->c[i].raw_data == NULL)
            return -1;
    }

    return 0;
}

int DS1054Z_chanRead(void *pScope, int channel){
    uint32_t ret_cnt;
    ViStatus vi_status;
    char tmc_size;
    char read_size_data_arg[16] = "%*c%*c%0d";
    Scope *s = (Scope*)pScope;
    ScopeChannel *c = &s->c[channel-1];
    viPrintf(s->vi, ":WAV:SOUR CHAN%d\n:WAV:DATA?\n", channel); // Ask for channel data
    //viPrintf(s->vi, ":WAV:SOUR CHAN%d\n:WAV:STAR 1\n:WAV:STOP %d\n:WAV:DATA?\n", channel, s->wave.max_data_stop); // Ask for channel data
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
        printf("fuck it, the DS1054Z is stupid");
        /*
        int available_channels;
        for (available_channels = 0;
        memcpy(&c->raw_data[0], &c->rd_buf[s->wave.header], sizeof(uint8_t) * (s->wave.max_data - s->wave.header));
        int raw_data_idx = s->wave.max_data - s->wave.header;
        int start = s->wave.max_data;
        int stop  = start * 2;
        for (; raw_data_idx < c->size_raw_data - s->wave.header;){
            viPrintf(s->vi, ":WAV:STAR %d\n:WAV:STOP %d\n:WAV:DATA?\n", start, stop); // Ask for channel data
            vi_status = viRead(s->vi, c->rd_buf, s->wave.max_data, (ViPUInt32)&ret_cnt); // Read X data bytes
            if (vi_status != VI_SUCCESS && vi_status != VI_SUCCESS_MAX_CNT)
                return -1;

            memcpy(&c->raw_data[raw_data_idx], &c->rd_buf[s->wave.header], sizeof(uint8_t) * (s->wave.max_data - s->wave.header));
            raw_data_idx += ret_cnt;
        }
        */
    }
    return 0;
}

void DS1054Z_run(void *pScope){
    Scope *s = (Scope*)pScope;
    // Run the scope
    viPrintf(s->vi, ":RUN\n");
    s->running = 1;
}

void DS1054Z_stop(void *pScope){
    Scope *s = (Scope*)pScope;
    // Stop the scope
    viPrintf(s->vi, ":STOP\n");
    s->running = 0;
}

void DS1054Z_setLongMemory(void *pScope){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":ACQ:MDEP %d\n", DS1054Z_MAX_MEM_DEPTH); // Set Memdepth to Long
    viPrintf(s->vi, ":WAV:MODE MAX\n"); // Set Waveform Point Mode to MAX
    viPrintf(s->vi, ":WAV:FORM BYTE\n"); // Set Waveform format to byte
}

void DS1054Z_unlockScope(void *pScope){
    return;
}

void DS1054Z_sampleRate(void *pScope){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":ACQ:SRAT?\n"); // Ask Sample rate
    viScanf(s->vi, "%t\n", &buf[0]);
    s->sample_rate = strtof(&buf[0], &p);
}

void DS1054Z_timeScale(void *pScope){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":TIM:SCAL?\n");
    viScanf(s->vi, "%t\n", buf);
    s->time_scale = strtof(&buf[0], &p);
}

void DS1054Z_timeOffset(void *pScope){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":TIM:OFFS?\n");
    viScanf(s->vi, "%t\n", buf);
    s->time_offset = strtof(&buf[0], &p);
}

void DS1054Z_chanVScale(void *pScope, int channel){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:SCAL?\n", channel); // Ask Chan V Scale
    viScanf(s->vi, "%t\n", &buf[0]);
    s->c[channel-1].Vscale = strtof(&buf[0], &p);
}

void DS1054Z_chanVOffset(void *pScope, int channel){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:OFFS?\n", channel); // Ask Chan V Offset
    viScanf(s->vi, "%t\n", &buf[0]);
    s->c[channel-1].Voffset = strtof(&buf[0], &p);
}

void DS1054Z_chanProbe(void *pScope, int channel){
    char *p;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:PROB?\n", channel); // Ask Chan probe
    viScanf(s->vi, "%t\n", &buf[0]);
    s->c[channel-1].probe = strtof(&buf[0], &p);
}

void DS1054Z_chanEnable(void *pScope, int channel){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:DISP ON\n", channel);
    s->c[channel-1].enabled = 1;
}

void DS1054Z_chanDisable(void *pScope, int channel){
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:DISP OFF\n", channel);
    s->c[channel-1].enabled = 0;
}

int DS1054Z_isChanEnabled(void *pScope, int channel){
    int enabled;
    char buf[256] = {0};
    Scope *s = (Scope*)pScope;
    viPrintf(s->vi, ":CHAN%d:DISP?\n", channel);
    viScanf(s->vi, "%t\n", buf);
    sscanf(buf, "%d", &enabled);
    s->c[channel - 1].enabled = enabled;
    return enabled;
}

void DS1054Z_printChannelInfo(void *pScope, int channel){
    Scope *s = (Scope*)pScope;
    printf("Sample rate : %fHz\n", s->sample_rate);
    printf("Time scale: %fs\n", s->time_scale);
    printf("Time offset: %fs\n", s->time_offset);

    printf("Channel Vscale : %fV\n", s->c[channel-1].Vscale);
    printf("Channel Voffset : %fV\n", s->c[channel-1].Voffset);
    printf("Channel probe : x%f\n", s->c[channel-1].probe);
}
