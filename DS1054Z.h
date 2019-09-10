#ifndef DS1054Z_H_INCLUDED
#define DS1054Z_H_INCLUDED

#include <stdint.h>
#include "scope.h"

/**
 Useful links:
 -------------

 - Programming Manual: http://int.rigol.com/File/TechDoc/20151218/MSO1000Z&DS1000Z_ProgrammingGuide_EN.pdf
**/

#define DS1054Z_ZOOM_FACTOR 2 // for graphics

#define DS1054Z_SCOPE_W 600
/**
 Oscilloscope height is set for (ADC) byte values between 25 and 225 IRL
 > Range of data is between 0 and 255 (see article)
 > Height of oscilloscope can be extended to 500
    - This adds 2 extra vertical divisions (one at the top and one at the bottom),
      which are not displayed IRL
**/
#define DS1054Z_SCOPE_H 400 // can be set to 500 but then the x2 zoom makes the window too big

#define DS1054Z_CHANNELS 4

#define DS1054Z_V_DIV 50 // pixels in vertical division
#define DS1054Z_H_DIV 50 // pixels in horizontal division

#define DS1054Z_BYTE_MAX 225 // from tests
#define DS1054Z_BYTE_MIN 25 // from tests
#define DS1054Z_BYTE_CENTER 125 // GND with no vertical offset

// Origin is top left
#define DS1054Z_ORIGIN_X 0
#define DS1054Z_ORIGIN_Y DS1054Z_SCOPE_H

/**
 Data received from waveform command

 Header data looks like this : "#9000001200"
 > Named TMC Blockheader
 > 11 bytes, all ASCII
 > "#9" -> 9 ASCII numbers following
 > "000001200" -> data length
**/
#define DS1054Z_HEADER_SIZE 11
#define DS1054Z_MAX_DATA_RUN 1200
#define DS1054Z_MAX_WAVE_DATA (250000 + DS1054Z_HEADER_SIZE) // in waveform format = byte
#define DS1054Z_MAX_DATA_STOP 24000000

#define DS1054Z_MAX_MEM_DEPTH 24000000


#define DS1054Z_COLOR_CH1 0xFF00EEFF
#define DS1054Z_COLOR_CH2 0xFFFFCC00
#define DS1054Z_COLOR_CH3 0xFFFF60FF
#define DS1054Z_COLOR_CH4 0xFFFF8000

int DS1054Z_initScope(void *pScope);

/* Write to scope functions */
void DS1054Z_setLongMemory(void *pScope);
// start / stop scope
void DS1054Z_stop(void *pScope);
void DS1054Z_run(void *pScope);
// Unlock scope
void DS1054Z_unlockScope(void *pScope);
// Enable / Disable channel
void DS1054Z_chanEnable(void *pScope, int channel); // needs channel argument
void DS1054Z_chanDisable(void *pScope, int channel); // needs channel argument

/* Read from scope functions */
// Scope time stuff
void DS1054Z_sampleRate(void *pScope);
void DS1054Z_timeScale(void *pScope);
void DS1054Z_timeOffset(void *pScope);
// Channel specific functions
int DS1054Z_chanRead(void *pScope, int channel); // needs channel argument
void DS1054Z_chanVScale(void *pScope, int channel); // needs channel argument
void DS1054Z_chanVOffset(void *pScope, int channel); // needs channel argument
void DS1054Z_chanProbe(void *pScope, int channel); // needs channel argument
int DS1054Z_isChanEnabled(void *pScope, int channel); // needs channel argument

/* Other functions */
// Basic printf displaying all channel info
void DS1054Z_printChannelInfo(void *pScope, int channel); // needs channel argument

#endif // DS1054Z_H_INCLUDED
