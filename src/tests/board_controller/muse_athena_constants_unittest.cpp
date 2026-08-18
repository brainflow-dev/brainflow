#include <gmock/gmock.h>

#include "muse_athena_constants.h"


TEST (MuseAthenaConstantsTest, EegScaleMatchesFourteenBitFullScaleRange)
{
    constexpr double max_14_bit_code = 16383.0;
    constexpr double full_scale_microvolts = 1450.0;

    EXPECT_NEAR (MUSE_ATHENA_EEG_SCALE_FACTOR * max_14_bit_code,
        full_scale_microvolts, 1e-12);
}
