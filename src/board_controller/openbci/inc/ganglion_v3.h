#pragma once

#include "ganglion.h"

#include <cstdint>

class GanglionV3 : public Ganglion
{
public:
    GanglionV3 (struct BrainFlowInputParams params);

    void decompress (struct GanglionLib::GanglionData *data, float *last_data, double *acceleration,
        double *package) override;
};