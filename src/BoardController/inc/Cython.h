#ifndef CYTHON
#define CYTHON

#include <math.h> 
#include <array>
#include <stdint.h>
#include "Board.h"
#include "spdlog/spdlog.h"
#include "DataBuffer.h"

#define SAMPLE_RATE 250.0
#define START_BYTE 0xA0
#define END_BYTE 0xC0
#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0


class Cython : public Board
{
    float eeg_scale = ADS1299_Vref/float((pow(2,23)-1))/ADS1299_gain*1000000.;
    float accel_scale = 0.002/(pow(2,4));

    private:

        int32_t cast_24bit_to_int32 (unsigned char *byteArray);
        int32_t cast_16bit_to_int32 (unsigned char *byteArray);

    protected:

        void read_thread ();

    public:
        // package num, 8 eeg channels, 3 accel channels
        Cython (const char *port_name) : Board (12, port_name){}

};

#endif
