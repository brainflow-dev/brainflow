#pragma once

#include "data_handler.h"
#include "filter_helpers.h"

namespace band_power_settings
{
    inline bool prepare_band_power (int fs, int apply, int &nfft, double low, double high,
        int mains, std::vector<band_power_helpers::Section> &sections, int &guard)
    {
        if (fs < 1 || fs > std::numeric_limits<int>::max () / 4 || apply < 0 || apply > 1 ||
            !std::isfinite (low) || !std::isfinite (high) || low < 0 || high < 0 ||
            low >= fs / 2.0 || high >= fs / 2.0 || (low > 0 && high > 0 && low >= high) ||
            mains < -1 || mains > 3)
            return false;
        if (nfft == 0)
        {
            if (get_nearest_power_of_two (fs, &nfft))
                return false;
            nfft = std::max (8, 2 * nfft);
        }
        if (nfft < 2 || (nfft & (nfft - 1)))
            return false;
        guard = 0;
        if (!apply)
            return true;
        double radius = 0.0;
        for (int i = 0; i < 2; i++)
        {
            const double center = i == 0 ? 50.0 : 60.0;
            // Automatic mains removal excludes the poorly conditioned region next to Nyquist.
            const bool enabled = mains == -1 ? center + 2 < 0.9 * fs / 2.0 : (mains & (1 << i));
            if (enabled &&
                !filter_helpers::design (3, fs, center - 2, center + 2, 4, 0, 0, sections, radius))
                return false;
        }
        if (low > 0 && high > 0)
        {
            if (!filter_helpers::design (2, fs, low, high, 4, 0, 0, sections, radius))
                return false;
        }
        else if (low > 0 || high > 0)
        {
            if (!filter_helpers::design (
                    low > 0 ? 1 : 0, fs, low > 0 ? low : high, 0, 4, 0, 0, sections, radius))
                return false;
        }
        return filter_helpers::settling_samples (sections, radius, guard);
    }
}
