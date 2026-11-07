#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include "data_filter.h"

static void reject (const std::function<void ()> &action)
{
    try
    {
        action ();
    }
    catch (const BrainFlowException &)
    {
        return;
    }
    throw std::runtime_error ("Expected invalid input to be rejected");
}

static void quality_tests ()
{
    BrainFlowArray<double, 2> data (3, 1024);
    for (int row = 0; row < 3; ++row)
        for (int i = 0; i < 1024; ++i)
            data.at (row, i) = (row + 1) * std::sin (2 * 3.141592653589793 * 10 * i / 128 + .1);
    SignalQualityOptions options;
    options.enable_clipping = true;
    options.lower_bound = -10;
    options.upper_bound = 10;
    options.min_channels = 2;
    data.at (1, 200) = 1000;
    auto result =
        DataFilter::get_avg_band_powers_with_quality (data, {2, 1, 0}, 128, false, options);
    if (!result.valid || result.used_channels != std::vector<int> ({2, 0}) ||
        result.rejected_channels != std::vector<int> ({1}) ||
        result.quality[1].flags != BRAINFLOW_QUALITY_CLIPPED || data.at (1, 200) != 1000)
        throw std::runtime_error ("Quality exclusion, channel identity or raw preservation failed");
    auto expected = DataFilter::get_avg_band_powers (data, {2, 0}, 128, false);
    std::unique_ptr<double[]> means (expected.first), variation (expected.second);
    for (int i = 0; i < 5; ++i)
        if (result.avg_band_powers[i] != means[i] || result.stddev_band_powers[i] != variation[i])
            throw std::runtime_error ("Quality-aware calculation changed the accepted-channel DSP");
    data.at (0, 100) = std::numeric_limits<double>::quiet_NaN ();
    result = DataFilter::get_avg_band_powers_with_quality (data, {0, 1, 2}, 128, false, options);
    if (result.valid || !result.avg_band_powers.empty () || !result.stddev_band_powers.empty ())
        throw std::runtime_error ("Insufficient channels exposed usable powers");
    auto quality = DataFilter::get_signal_quality (data, {0, 1, 2}, 128, options);
    if (quality.quality[0].flags != BRAINFLOW_QUALITY_NONFINITE)
        throw std::runtime_error ("Nonfinite diagnosis missing");
    reject ([&] { DataFilter::get_signal_quality (data, {0, 0}, 128); });
    reject ([&] { DataFilter::get_signal_quality (data, {-1}, 128); });
    options.min_channels = 0;
    reject (
        [&] {
            DataFilter::get_avg_band_powers_with_quality (data, {0, 1}, 128, false, options);
        });
}

int main ()
try
{
    quality_tests ();
    BrainFlowArray<double, 2> data (2, 256);
    reject ([&] { data.at (-1, 0); });
    reject ([&] { data.at (0, 256); });
    reject ([&] { data.get_address (-1); });
    reject ([&] { data[-1]; });
    reject ([&] { BrainFlowArray<double, 2> invalid (-1, 256); });
    reject (
        [&] {
            DataFilter::get_csp (
                BrainFlowArray<double, 3> (2, 2, 8), BrainFlowArray<double, 1> (1));
        });
    reject ([&] { DataFilter::perform_ica (data, 2, {-1, 1}); });
    reject ([&] { DataFilter::get_avg_band_powers (data, {-1}, 256, false); });
    reject ([&] { DataFilter::perform_fft (data.get_raw_ptr (), 256, 0, nullptr); });
    reject ([&] { DataFilter::get_psd (data.get_raw_ptr (), 256, 256, 0, nullptr); });
    reject ([&] { DataFilter::perform_wavelet_transform (data.get_raw_ptr (), 256, 3, -1); });
    reject (
        [&] {
            DataFilter::perform_inverse_wavelet_transform (
                std::vector<double> (4), {2, 2}, 8, 3, 2);
        });
    for (int original_length :
        {4, std::numeric_limits<int>::max () / 2, std::numeric_limits<int>::max ()})
        reject (
            [&]
            {
                DataFilter::perform_inverse_wavelet_transform (
                    std::vector<double> (2), {1, 1}, original_length, 0, 1);
            });

    std::vector<double> input (256);
    for (size_t i = 0; i < input.size (); ++i)
        input[i] = std::sin (0.1 * i);
    auto transform =
        DataFilter::perform_wavelet_transform (input.data (), (int)input.size (), 3, 3);
    std::vector<int> lengths (transform.second, transform.second + 4);
    int total = 0;
    for (int length : lengths)
        total += length;
    std::vector<double> coefficients (transform.first, transform.first + total);
    delete[] transform.first;
    delete[] transform.second;
    auto restored =
        DataFilter::perform_inverse_wavelet_transform (coefficients, lengths, 256, 3, 3);
    for (size_t i = 0; i < input.size (); ++i)
        if (std::abs (input[i] - restored[i]) > 1e-10)
            throw std::runtime_error ("Wavelet round trip failed");
    lengths[0]++;
    reject (
        [&] { DataFilter::perform_inverse_wavelet_transform (coefficients, lengths, 256, 3, 3); });
    std::cout << "Signal-processing binding validation passed." << std::endl;
    return 0;
}
catch (const std::exception &error)
{
    std::cerr << "Signal-processing binding validation failed: " << error.what () << std::endl;
    return 1;
}
