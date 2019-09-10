#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scope.h"

int SCOPE_openVISA(Scope *pScope){
    ViChar buffer[VI_FIND_BUFLEN];
    ViRsrc matches = buffer;
    ViUInt32 nmatches;
    ViFindList vi_list;
    ViStatus vi_status;

    // Initialise/Open VISA communication, copied from example program
    viOpenDefaultRM(&pScope->defaultRM);

    // Acquire USB resource of VISA, copied from example program
    viFindRsrc(pScope->defaultRM, "USB?*", &vi_list, &nmatches, matches);
    vi_status = viOpen(pScope->defaultRM, matches, VI_NULL, VI_NULL, &pScope->vi);
    if (vi_status != VI_SUCCESS){
        return -1;
    }

    // Change timeout
    //viSetAttribute(pScope->vi, VI_ATTR_TMO_VALUE, 150); // default is 2000ms, see visa prog manual http://www.ni.com/pdf/manuals/370132c.pdf
    return 0;
}

void SCOPE_getID(Scope *pScope){
    memset(pScope->deviceID, 0, SCOPE_ID_BUFF_SIZE);
    viPrintf(pScope->vi, "*IDN?\n"); // Ask ID
    viScanf(pScope->vi, "%t\n", pScope->deviceID);
}

void SCOPE_closeVISA(Scope *pScope){
    // Close VISA
    viClose(pScope->vi);
    viClose(pScope->defaultRM);
}

void SCOPE_freeScope(Scope *pScope){
    int i;
    for (i = 0; i < pScope->channels; i++){
        free(pScope->c[i].raw_data);
        free(pScope->c[i].rd_buf);
    }
    free(pScope->c);
    free(pScope);
}
