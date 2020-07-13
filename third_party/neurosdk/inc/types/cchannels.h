#ifndef CCHANNELS_H
#define CCHANNELS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cdevice.h"
#include "clistener.h"
#include "lib_export.h"
#include <stdbool.h>
#include <stddef.h>


    typedef struct _AnyChannel AnyChannel;
    typedef struct _IntChannel IntChannel;
    typedef struct _DoubleChannel DoubleChannel;
    typedef struct _BridgeDoubleChannel BridgeDoubleChannel;

    typedef enum _Filter
    {
        LowPass_1Hz_SF125,
        LowPass_1Hz_SF125_Reverse,
        LowPass_5Hz_SF125,
        LowPass_5Hz_SF125_Reverse,
        LowPass_15Hz_SF125,
        LowPass_15Hz_SF125_Reverse,
        LowPass_27Hz_SF125,
        LowPass_27Hz_SF125_Reverse,
        LowPass_30Hz_SF250,
        LowPass_30Hz_SF250_Reverse,

        HighPass_2Hz_SF250,
        HighPass_2Hz_SF250_Reverse,
        HighPass_3Hz_SF125,
        HighPass_3Hz_SF125_Reverse,
        HighPass_5Hz_SF125,
        HighPass_5Hz_SF125_Reverse,
        HighPass_11Hz_SF125,
        HighPass_11Hz_SF125_Reverse,

        BandStop_45_55Hz_SF250
    } Filter;

#ifdef __cplusplus
}
#endif

#endif // CCHANNELS_H
