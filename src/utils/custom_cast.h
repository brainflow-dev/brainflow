#ifndef CUSTOM_CAST
#define CUSTOM_CAST

#include <bitset>
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

// this function is specific to the ganglion board, as it deals with its quirks
// input array is an array of 0 and 1 (not the chartacters '0' and '1',
// but 8-bit unsigned integers 0 and 1)
template <unsigned int N> inline int32_t cast_ganglion_bits_to_int32 (unsigned char *array)
{
    // need to convert to std::string to feed the bitset
    std::string bitstring ((char *)array, N);
    // initialize the bit set with the string. specify custom 0 and 1 characters
    // (must convert ints 0 and 1 to their respective chars)
    std::bitset<N> bits (bitstring, 0, N, (char)0, (char)1);

    int32_t result = 0;

    // check the most significant bit to figure out if it's a negative value
    bool negative = bits.test (N - 1);
    if (negative)
    {
        // 2's complement: to get a negative value, we flip the bits,
        // add 1 to the value, then take the negative.
        // because of a quirk in ganglion data, we need to add 2 to the value
        bits.flip ();
        result = -bits.to_ulong () - 2;
    }
    else
    {
        result = bits.to_ulong ();
    }

    return result;
}

#endif
