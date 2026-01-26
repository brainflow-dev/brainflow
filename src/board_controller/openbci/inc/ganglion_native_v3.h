#pragma once

#include "ganglion_native.h"
#include <cstdint>

class GanglionNativeV3 : public GanglionNative
{
public:
    GanglionNativeV3 (struct BrainFlowInputParams params);

    void decompress (const uint8_t *data, double *package) override;
};