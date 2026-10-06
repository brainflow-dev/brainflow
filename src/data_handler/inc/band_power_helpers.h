#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "DspFilters/Dsp.h"

namespace band_power_helpers
{
    struct Section
    {
        double b0, b1, b2, a1, a2;
    };

    // Keep the filter design in DSPFilters, but initialize the SOS states separately for
    // each direction. Reusing the forward pass's final state produces a boundary transient.
    inline void append_filter (
        Dsp::Cascade &filter, std::vector<Section> &sections, double &pole_radius)
    {
        for (int i = 0; i < filter.getNumStages (); i++)
        {
            const Dsp::Biquad &stage = filter[i];
            const double a0 = stage.getA0 ();
            sections.push_back ({stage.getB0 () / a0, stage.getB1 () / a0, stage.getB2 () / a0,
                stage.getA1 () / a0, stage.getA2 () / a0});
        }
        for (const auto &pair : filter.getPoleZeros ())
        {
            pole_radius = std::max (pole_radius, std::abs (pair.poles.first));
            pole_radius = std::max (pole_radius, std::abs (pair.poles.second));
        }
    }

    inline bool filter_direction (
        std::vector<double> &data, const std::vector<Section> &sections, bool steady_state = true)
    {
        for (const Section &s : sections)
        {
            // Steady state for a constant input equal to the endpoint, in transposed
            // direct form II. Each section sees the preceding section's endpoint gain.
            const double initial = steady_state ? data.front () : 0.0;
            const double output = initial * (s.b0 + s.b1 + s.b2) / (1.0 + s.a1 + s.a2);
            double z1 = output - s.b0 * initial;
            double z2 = s.b2 * initial - s.a2 * output;
            for (double &sample : data)
            {
                const double filtered = s.b0 * sample + z1;
                z1 = s.b1 * sample - s.a1 * filtered + z2;
                z2 = s.b2 * sample - s.a2 * filtered;
                if (!std::isfinite (filtered))
                {
                    return false;
                }
                sample = filtered;
            }
        }
        return true;
    }

    inline bool filter_with_padding (
        std::vector<double> &data, const std::vector<Section> &sections, int guard)
    {
        if (data.empty () || guard < 0)
            return false;
        const size_t pad = std::min ((size_t)guard, data.size () - 1);
        const size_t size = data.size ();
        std::vector<double> padded (size + 2 * pad);
        std::copy (data.begin (), data.end (), padded.begin () + pad);
        for (size_t i = 0; i < pad; i++)
        {
            padded[pad - 1 - i] = data.front () + (data.front () - data[i + 1]);
            padded[pad + size + i] = data.back () + (data.back () - data[size - 2 - i]);
        }
        if (!filter_direction (padded, sections))
        {
            return false;
        }
        std::reverse (padded.begin (), padded.end ());
        if (!filter_direction (padded, sections))
        {
            return false;
        }
        std::reverse (padded.begin (), padded.end ());
        std::copy (padded.begin () + pad, padded.begin () + pad + size, data.begin ());
        return true;
    }
}
