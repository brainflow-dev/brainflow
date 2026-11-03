#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
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

int main ()
{
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

    BrainFlowArray<double, 2> reref_data (4, 3);
    double values[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            reref_data.at (i, j) = values[i * 3 + j];
    DataFilter::rereference_channels (reref_data, {0, 1}, {0, 1});
    for (int j = 0; j < 3; j++)
        if (std::abs (reref_data.at (0, j) + 1.5) > 1e-12 ||
            std::abs (reref_data.at (1, j) - 1.5) > 1e-12 ||
            std::abs (reref_data.at (2, j) - values[6 + j]) > 1e-12)
            throw std::runtime_error ("Two-channel average reference failed");

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            reref_data.at (i, j) = values[i * 3 + j];
    DataFilter::rereference_channels (reref_data, {0, 1, 2, 3}, {1});
    for (int j = 0; j < 3; j++)
        if (std::abs (reref_data.at (1, j)) > 1e-12 ||
            std::abs (reref_data.at (0, j) - (values[j] - values[3 + j])) > 1e-12)
            throw std::runtime_error ("Single-channel reference failed");

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            reref_data.at (i, j) = values[i * 3 + j];
    DataFilter::rereference_channels (reref_data, {0, 1, 2, 3}, {0, 1, 2, 3});
    for (int j = 0; j < 3; j++)
    {
        double mean = (values[j] + values[3 + j] + values[6 + j] + values[9 + j]) / 4.0;
        for (int i = 0; i < 4; i++)
            if (std::abs (reref_data.at (i, j) - (values[i * 3 + j] - mean)) > 1e-12)
                throw std::runtime_error ("Common average reference failed");
    }
    reject ([&] { DataFilter::rereference_channels (reref_data, {0}, {4}); });
    reject ([&] { DataFilter::rereference_channels (reref_data, {0, 0}, {1}); });

    std::cout << "Signal-processing binding validation passed." << std::endl;
}
