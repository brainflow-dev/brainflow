#pragma once

#include "band_power_helpers.h"
#include "common_data_handler_helpers.h"

namespace filter_helpers
{
    using band_power_helpers::Section;

    template <class Design>
    struct AccessibleFilter : Dsp::FilterDesign<Design, 1>
    {
        Dsp::Cascade &cascade ()
        {
            return this->m_design;
        }
    };

    template <class Design>
    void append (const Dsp::Params &params, std::vector<Section> &sections, double &radius)
    {
        AccessibleFilter<Design> filter;
        filter.setParams (params);
        band_power_helpers::append_filter (filter.cascade (), sections, radius);
    }

    // kind: lowpass=0, highpass=1, bandpass=2, bandstop=3.
    inline bool design (int kind, int fs, double low, double high, int order, int type,
        double ripple, std::vector<Section> &sections, double &radius)
    {
        if (fs < 1 || order < 1 || order > 8 || type < 0 || type > 5 || kind < 0 || kind > 3 ||
            !std::isfinite (low) || low <= 0.0 || low >= fs / 2.0 ||
            (kind >= 2 && (!std::isfinite (high) || high <= low || high >= fs / 2.0)) ||
            (type % 3 == 1 && (!std::isfinite (ripple) || ripple <= 0.0)))
            return false;
        Dsp::Params params;
        params[0] = fs;
        params[1] = order;
        params[2] = kind < 2 ? low : (low + high) / 2.0;
        params[3] = kind < 2 ? ripple : high - low;
        params[4] = ripple;
#define APPEND_FAMILY(FAMILY)                                                                      \
    switch (kind)                                                                                  \
    {                                                                                              \
        case 0:                                                                                    \
            append<Dsp::FAMILY::Design::LowPass<8>> (params, sections, radius);                    \
            break;                                                                                 \
        case 1:                                                                                    \
            append<Dsp::FAMILY::Design::HighPass<8>> (params, sections, radius);                   \
            break;                                                                                 \
        case 2:                                                                                    \
            append<Dsp::FAMILY::Design::BandPass<8>> (params, sections, radius);                   \
            break;                                                                                 \
        case 3:                                                                                    \
            append<Dsp::FAMILY::Design::BandStop<8>> (params, sections, radius);                   \
            break;                                                                                 \
    }
        switch (type % 3)
        {
            case 0:
                APPEND_FAMILY (Butterworth);
                break;
            case 1:
                APPEND_FAMILY (ChebyshevI);
                break;
            case 2:
                APPEND_FAMILY (Bessel);
                break;
        }
#undef APPEND_FAMILY
        if (!std::isfinite (radius) || radius >= 1.0)
            return false;
        for (const auto &s : sections)
            if (!std::isfinite (s.b0) || !std::isfinite (s.b1) || !std::isfinite (s.b2) ||
                !std::isfinite (s.a1) || !std::isfinite (s.a2) || 1 + s.a1 + s.a2 <= 0)
                return false;
        return true;
    }

    // Measure the complete cascade's impulse tail, instead of assigning a unit-amplitude
    // transient to every pole. This remains an estimate, not a bound for arbitrary signals.
    inline bool settling_samples (const std::vector<Section> &sections, double radius, int &guard)
    {
        guard = 0;
        if (sections.empty ())
            return true;
        if (radius == 0.0)
        {
            guard = 1;
            return true;
        }
        if (!(radius > 0 && radius < 1))
            return false;
        const double horizon = std::ceil (std::log (1e-9) / std::log (radius));
        if (!std::isfinite (horizon) || horizon > 1000000)
            return false;
        std::vector<double> impulse (std::max (32, (int)horizon), 0.0);
        impulse[0] = 1.0;
        if (!band_power_helpers::filter_direction (impulse, sections, false))
            return false;
        double total = 0.0;
        for (double value : impulse)
            total += std::abs (value);
        double tail = 0.0;
        for (int i = (int)impulse.size () - 1; i >= 0; i--)
        {
            tail += std::abs (impulse[i]);
            if (tail > 1e-3 * total)
            {
                guard = i + 1;
                break;
            }
        }
        return true;
    }
}
