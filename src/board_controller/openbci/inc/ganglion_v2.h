#pragma once

#include "ganglion.h"

#include <cstdint>

class GanglionV2 : public Ganglion
{
public:
    GanglionV2 (struct BrainFlowInputParams params);

    void decompress (struct GanglionLib::GanglionData *data, float *last_data, double *acceleration,
        double *package) override;
};