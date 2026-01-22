#pragma once

#include "ganglion_native.h"

#include <cstdint>

class GanglionNativeV2 : public GanglionNative
{
public:
    GanglionNativeV2 (struct BrainFlowInputParams params);

    void decompress (const uint8_t *data, double *package) override;
};