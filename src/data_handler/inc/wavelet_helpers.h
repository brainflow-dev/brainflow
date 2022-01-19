#pragma once

#include <string>
#include <vector>


inline bool validate_wavelet (const char *wavelet)
{
    // https://github.com/rafat/wavelib/wiki/wave-object
    std::vector<std::string> supported_wavelets;
    supported_wavelets.push_back (std::string ("haar"));
    for (int i = 1; i <= 15; i++)
    {
        supported_wavelets.push_back (std::string ("db") + std::to_string (i));
    }
    for (int i = 2; i <= 10; i++)
    {
        supported_wavelets.push_back (std::string ("sym") + std::to_string (i));
    }
    for (int i = 1; i <= 5; i++)
    {
        supported_wavelets.push_back (std::string ("coif") + std::to_string (i));
    }
    supported_wavelets.push_back ("bior1.1");
    supported_wavelets.push_back ("bior1.3");
    supported_wavelets.push_back ("bior1.5");
    supported_wavelets.push_back ("bior2.2");
    supported_wavelets.push_back ("bior2.4");
    supported_wavelets.push_back ("bior2.6");
    supported_wavelets.push_back ("bior2.8");
    supported_wavelets.push_back ("bior3.1");
    supported_wavelets.push_back ("bior3.3");
    supported_wavelets.push_back ("bior3.5");
    supported_wavelets.push_back ("bior3.7");
    supported_wavelets.push_back ("bior3.9");
    supported_wavelets.push_back ("bior4.4");
    supported_wavelets.push_back ("bior5.5");
    supported_wavelets.push_back ("bior6.8");

    bool is_supported = false;
    for (auto cur_wavelet : supported_wavelets)
    {
        if (std::string (wavelet) == cur_wavelet)
        {
            is_supported = true;
            break;
        }
    }
    return is_supported;
}
