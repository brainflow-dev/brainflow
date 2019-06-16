#ifndef CUSTOM_CAST
#define CUSTOM_CAST

#include <stdint.h>


inline int32_t cast_24bit_to_int32 (unsigned char *byte_array)
{
    int32_t new_int =
        (((0xFF & byte_array[0]) << 16) | ((0xFF & byte_array[1]) << 8) | (0xFF & byte_array[2]));
    if ((new_int & 0x00800000) > 0)
        new_int |= 0xFF000000;
    else
        new_int &= 0x00FFFFFF;
    return new_int;
}

inline int32_t cast_16bit_to_int32 (unsigned char *byte_array)
{
    int32_t new_int = (((0xFF & byte_array[0]) << 8) | (0xFF & byte_array[1]));
    if ((new_int & 0x00008000) > 0)
        new_int |= 0xFFFF0000;
    else
        new_int &= 0x0000FFFF;
    return new_int;
}

inline void uchar_to_bits (unsigned char c, unsigned char *bits)
{
    for (int i = sizeof (unsigned char) * 8; i; c >>= 1)
    {
        bits[--i] = c & 1;
    }
}

// here input arg is an array of 0 and 1
inline int32_t cast_18bit_to_int32 (unsigned char *array)
{
    int prefix = 0;
    if (array[2] & 0x01 > 0)
    {
        prefix = 0b11111111111111;
    }
    return (prefix << 18) | (array[0] << 16) | (array[1] << 8) | array[2];
}

// here input arg is an array of 0 and 1
inline int32_t cast_19bit_to_int32 (unsigned char *array)
{
    int prefix = 0;
    if (array[2] & 0x01 > 0)
    {
        prefix = 0b1111111111111;
    }
    return (prefix << 19) | (array[0] << 16) | (array[1] << 8) | array[2];
}

#endif
