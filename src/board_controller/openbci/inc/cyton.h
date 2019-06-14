#ifndef CYTON
#define CYTON

#include <math.h>
#include <stdint.h>

#include "data_buffer.h"
#include "openbci_serial_board.h"
#include "spdlog/spdlog.h"

#define SAMPLE_RATE 250.0
#define START_BYTE 0xA0
#define END_BYTE 0xC0
#define ADS1299_Vref 4.5
#define ADS1299_gain 24.0

class Cyton : public OpenBCISerialBoard
{
    float eeg_scale = ADS1299_Vref / float((pow (2, 23) - 1)) / ADS1299_gain * 1000000.;
    float accel_scale = 0.002 / (pow (2, 4));

protected:
    void read_thread ();

public:
    // package num, 8 eeg channels, 3 accel channels
    Cyton (const char *port_name) : OpenBCISerialBoard (12, port_name)
    {
    }

    int get_board_id ()
    {
        return GANGLION_BOARD;
    }
};

#endif
