#ifndef __KINGSONG_H
#define __KINGSONG_H

/*define constants for specific KS wheel models 
* add here when adding support for a new model
*/
#define KS_14D_MAXCURRENT   35
#define KS_14D_CRITTEMP     65
#define KS_14D_WARNTEMP     50
#define KS_14D_BATTVOLT     67
#define KS_14D_BATTWARN     40

#define KS_14S_MAXCURRENT   35
#define KS_14S_CRITTEMP     65
#define KS_14S_WARNTEMP     50
#define KS_14S_BATTVOLT     67
#define KS_14S_BATTWARN     25

#define KS_16S_MAXCURRENT   40
#define KS_16S_CRITTEMP     65
#define KS_16S_WARNTEMP     50
#define KS_16S_BATTVOLT     67
#define KS_16S_BATTWARN     25

#define KS_16X_MAXCURRENT   45
#define KS_16X_CRITTEMP     65
#define KS_16X_WARNTEMP     50
#define KS_16X_BATTVOLT     84
#define KS_16X_BATTWARN     25


#include <TTGO.h>
#include "callback.h"

struct Wheel_constants
{
    byte maxcurrent;
    byte crittemp;
    byte warntemp;
    byte battvolt;
    byte battwarn;
};

void decodeKS(byte KSData[]);
void initks();

extern float wheeldata[];

#endif /* __KINGSONG */