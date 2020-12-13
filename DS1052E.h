#ifndef DS1052E_H_INCLUDED
#define DS1052E_H_INCLUDED

#include <stdint.h>
#include "scope.h"

/**
 Useful links:
 -------------

 - Programming Manual: https://www.batronix.com/pdf/Rigol/ProgrammingGuide/DS1000DE_ProgrammingGuide_EN.pdf
 - Rigol article 1: https://rigol.desk.com/customer/en/portal/articles/2269119-how-do-i-format-the-data-returned-from-a-ds1000e-d-series-scope-
 - Rigol article 2: https://rigol.desk.com/customer/en/portal/articles/2269115-how-do-i-retrieve-data-using-usb-control-for-the-ds1000e-d-series-of-scopes-
**/

#define DS1052E_ZOOM_FACTOR 3 // for graphics

#define DS1052E_SCOPE_W 300
/**
 Oscilloscope height is set for (ADC) byte values between 25 and 225 IRL
 > Range of data is between 15 and 240 (see article)
 > Height of oscilloscope can be extended to 250
    - This adds 2 extra vertical divisions (one at the top and one at the bottom),
      which are not displayed IRL
      If you choose to show those extra vertical divisions be aware that the signal may
      be clipping to the highest/lowest values!
**/
#define DS1052E_SCOPE_H 250

#define DS1052E_CHANNELS 2

#define DS1052E_V_DIV 25 // pixels in vertical division
#define DS1052E_H_DIV 25 // pixels in horizontal division

#define DS1052E_BYTE_MAX 15 // from article
#define DS1052E_BYTE_MIN 240 // from article
#define DS1052E_BYTE_CENTER 125 // GND with no vertical offset

// Origin is top left
#define DS1052E_ORIGIN_X 0
#define DS1052E_ORIGIN_Y 0

/**
 Data received from waveform command
 > Waveform command can only send a max of 1024 bytes
 > in RUN mode : 610 bytes -> 600 raw data + 10 header bytes
 > in STOP mode + long mem depth :
    - Read data through multiple reads of 1024 bytes
    - If 1 channel enabled, then 1M samples
    - If 2 channels enabled, then 512k samples per channel
    - First read contains 10 byte header
    - 10 byte header is included in the total 1M (or 512K), so the actual samples is 1M-10 bytes
    - Following notes from the articles are incorrect: <- I tested the commands through NI VISA Interactive Control
        - When the scope is in stop mode, which is indicated by a red LED lighting the Stop/Run button,
            the scope will return 610 data points the first query. Subsequent queries will return data
            sets of 8k, 16k, 512k, or 1M points depending on the configuration.
        - NOTE: The first iteration of a data query, or request, performed in stop mode,
            indicated by a red Run/Stop button, will also return 610 data points.
            You can simply write these values to an unused storage location or delete them.

 Header data looks like this : "#800000600"
 > 10 bytes, all ASCII
 > "#8" -> 8 ASCII numbers following ?
 > "00000600" -> data length
**/
#define DS1052E_HEADER_SIZE 10
#define DS1052E_MAX_DATA_RUN 600
#define DS1052E_MAX_WAVE_DATA 1024 // From programming manual
#define DS1052E_MAX_DATA_STOP 1048576


#define DS1052E_COLOR_CH1 0xFF00EEFF
#define DS1052E_COLOR_CH2 0xFFFFCC00

int DS1052E_initScope(void *pScope);

/* Write to scope functions */
void DS1052E_setLongMemory(void *pScope);
// start / stop scope
void DS1052E_stop(void *pScope);
void DS1052E_run(void *pScope);
// Unlock scope
void DS1052E_unlockScope(void *pScope);
// Enable / Disable channel
void DS1052E_chanEnable(void *pScope, int channel); // needs channel argument
void DS1052E_chanDisable(void *pScope, int channel); // needs channel argument

/* Read from scope functions */
// Scope time stuff
void DS1052E_sampleRate(void *pScope);
void DS1052E_timeScale(void *pScope);
void DS1052E_timeOffset(void *pScope);
// Channel specific functions
int DS1052E_chanRead(void *pScope, int channel); // needs channel argument
void DS1052E_chanVScale(void *pScope, int channel); // needs channel argument
void DS1052E_chanVOffset(void *pScope, int channel); // needs channel argument
void DS1052E_chanProbe(void *pScope, int channel); // needs channel argument
int DS1052E_isChanEnabled(void *pScope, int channel); // needs channel argument

/* Other functions */
// Basic printf displaying all channel info
void DS1052E_printChannelInfo(void *pScope, int channel); // needs channel argument


#endif // DS1052_H_INCLUDED
