#include "Cython.h"
#include "TimeStamp.h"

int32_t Cython::cast_24bit_to_int32 (unsigned char *byte_array)
{     
    int32_t new_int = (  
        ((0xFF & byte_array[0]) << 16) |  
        ((0xFF & byte_array[1]) << 8) |   
        (0xFF & byte_array[2])  
    );  
    if ((new_int & 0x00800000) > 0)
        new_int |= 0xFF000000;  
    else
        new_int &= 0x00FFFFFF;  
    return new_int;  
}

int32_t Cython::cast_16bit_to_int32 (unsigned char *byte_array) {
    int32_t new_int = (
        ((0xFF & byte_array[0]) << 8) |
        (0xFF & byte_array[1])
    );
    if ((new_int & 0x00008000) > 0)
        new_int |= 0xFFFF0000;
    else
        new_int &= 0x0000FFFF;
    return new_int;
}

void Cython::read_thread ()
{
    /*
        Byte 1: 0xA0
        Byte 2: Sample Number
        Bytes 3-5: Data value for EEG channel 1
        Bytes 6-8: Data value for EEG channel 2
        Bytes 9-11: Data value for EEG channel 3
        Bytes 12-14: Data value for EEG channel 4
        Bytes 15-17: Data value for EEG channel 5
        Bytes 18-20: Data value for EEG channel 6
        Bytes 21-23: Data value for EEG channel 6
        Bytes 24-26: Data value for EEG channel 8
        Aux Data Bytes 27-32: 6 bytes of data
        Byte 33: 0xCX where X is 0-F in hex
    */
    int res;
    unsigned char b[32];
    while (keep_alive)
    {
        // check start byte
        res = read (port_descriptor, b, 1);
        if (res != 1)
        {
            logger->error ("unable to read 1 byte");
            continue;
        }
        if (b[0] != START_BYTE)
            continue;

        // check end byte
        res = read (port_descriptor, b, 32);
        if (res != 32)
        {
            logger->error ("unable to read 32 bytes");
            continue;
        }
        if (b[res] != END_BYTE)
            logger->debug ("Wrong end byte, found {}, required {}", b[res], END_BYTE);
        else
            continue;

        float package[12];
        package[0] = (float) b[0];
        for (int i = 0; i < 8; i++)
        {
            package[i+1] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
        }
        package[9] = accel_scale * cast_16bit_to_int32 (b + 25);
        package[10] = accel_scale * cast_16bit_to_int32 (b + 27);
        package[11] = accel_scale * cast_16bit_to_int32 (b + 29);

        db->add_data (get_timestamp (), package);
    }
}