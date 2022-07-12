#pragma once

#include <string>
#include <vector>

#include "brainflow_constants.h"


inline std::string get_wavelet_name (int wavelet)
{
    std::string wavelet_names[] = {"haar", "db1", "db2", "db3", "db4", "db5", "db6", "db7", "db8",
        "db9", "db10", "db11", "db12", "db13", "db14", "db15", "bior1.1", "bior1.3", "bior1.5",
        "bior2.2", "bior2.4", "bior2.6", "bior2.8", "bior3.1", "bior3.3", "bior3.5", "bior3.7",
        "bior3.9", "bior4.4", "bior5.5", "bior6.8", "coif1", "coif2", "coif3", "coif4", "coif5",
        "sym2", "sym3", "sym4", "sym5", "sym6", "sym7", "sym8", "sym9", "sym10"};

    if ((wavelet < (int)WaveletTypes::FIRST_WAVELET) || (wavelet > (int)WaveletTypes::LAST_WAVELET))
    {
        return "";
    }

    return wavelet_names[wavelet];
}

inline std::string get_wavelet_denoising_type (int denoising_type)
{
    if (denoising_type == (int)WaveletDenoisingTypes::VISUSHRINK)
    {
        return "visushrink";
    }
    else if (denoising_type == (int)WaveletDenoisingTypes::SURESHRINK)
    {
        return "sureshrink";
    }
    else
    {
        return "";
    }
}

inline std::string get_threshold_type (int threshold_type)
{
    if (threshold_type == (int)ThresholdTypes::SOFT)
    {
        return "soft";
    }
    else if (threshold_type == (int)ThresholdTypes::HARD)
    {
        return "hard";
    }
    else
    {
        return "";
    }
}

inline std::string get_extension_type (int extenstion_type)
{
    if (extenstion_type == (int)WaveletExtensionTypes::SYMMETRIC)
    {
        return "sym";
    }
    else if (extenstion_type == (int)WaveletExtensionTypes::PERIODIC)
    {
        return "per";
    }
    else
    {
        return "";
    }
}

inline std::string get_noise_estimation_type (int noise_est_type)
{
    if (noise_est_type == (int)NoiseEstimationLevelTypes::FIRST_LEVEL)
    {
        return "first";
    }
    else if (noise_est_type == (int)NoiseEstimationLevelTypes::ALL_LEVELS)
    {
        return "all";
    }
    else
    {
        return "";
    }
}
