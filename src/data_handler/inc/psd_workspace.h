#pragma once

#include "brainflow_constants.h"
#include "common_data_handler_helpers.h"
#include "data_handler.h"
#include "kiss_fftr.h"
#include <new>
#include <vector>

class PsdWorkspace
{
    kiss_fftr_cfg cfg;
    int nfft, fs;
    std::vector<double> weights, input, result;
    std::vector<kiss_fft_cpx> output;
    static const int ok = (int)BrainFlowExitCodes::STATUS_OK;
    static const int invalid = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;

public:
    PsdWorkspace (int size, int sampling_rate)
        : cfg (nullptr)
        , nfft (size)
        , fs (sampling_rate)
        , weights (size)
        , input (size)
        , result (size / 2 + 1)
        , output (size / 2 + 1)
    {
        cfg = kiss_fftr_alloc (nfft, 0, 0, 0);
        if (!cfg)
            throw std::bad_alloc ();
    }
    ~PsdWorkspace ()
    {
        kiss_fftr_free (cfg);
    }
    PsdWorkspace (const PsdWorkspace &) = delete;
    PsdWorkspace &operator= (const PsdWorkspace &) = delete;

    int prepare (int window)
    {
        int res = get_window (window, nfft, weights.data ());
        if (res != ok)
            return res;
        double energy = 0.0;
        for (double value : weights)
            energy += value * value;
        if (!(energy > 0.0))
            return invalid;
        const double denominator = std::sqrt ((double)fs * energy);
        for (double &value : weights)
            value /= denominator;
        return ok;
    }
    int process (const double *data, int n, int overlap, double *amplitudes, double *frequencies)
    {
        if (!amplitudes || !frequencies || n < nfft || overlap < 0 || overlap >= nfft ||
            !finite_signal (data, n))
            return invalid;
        std::fill (result.begin (), result.end (), 0.0);
        const int step = nfft - overlap;
        const int count = 1 + (n - nfft) / step;
        for (int segment = 0; segment < count; segment++)
        {
            const size_t start = (size_t)segment * step;
            // Normalize before the transform to avoid overflowing an otherwise representable PSD.
            for (int i = 0; i < nfft; i++)
                input[i] = data[start + i] * weights[i];
            kiss_fftr (cfg, input.data (), output.data ());
            for (int i = 0; i <= nfft / 2; i++)
            {
                const double magnitude = std::hypot (output[i].r, output[i].i);
                const double factor = (i == 0 || i == nfft / 2) ? 1.0 : 2.0;
                const double averaged_magnitude = magnitude / std::sqrt ((double)count);
                const double value = averaged_magnitude * averaged_magnitude * factor;
                if (!std::isfinite (value))
                    return invalid;
                result[i] += value;
                if (!std::isfinite (result[i]))
                    return invalid;
            }
        }
        std::copy (result.begin (), result.end (), amplitudes);
        for (int i = 0; i <= nfft / 2; i++)
            frequencies[i] = i * ((double)fs / nfft);
        return ok;
    }
};
