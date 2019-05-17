#ifndef CUSTOM_CAST
#define CUSTOM_CAST

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

#endif