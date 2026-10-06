#include <cmath>
#include <iostream>

#include "data_filter.h"

int main ()
{
    // Two simultaneous mixtures of non-Gaussian sources: rows are channels, columns are samples.
    const int samples = 1024;
    const double pi = std::acos (-1.0);
    BrainFlowArray<double, 2> data (2, samples);
    for (int i = 0; i < samples; i++)
    {
        double t = i / 256.0;
        double first = std::sin (2.0 * pi * 7.0 * t);
        double second = std::pow (std::sin (2.0 * pi * 13.0 * t), 3);
        data.at (0, i) = first + 0.3 * second;
        data.at (1, i) = 0.2 * first + second;
    }
    try
    {
        auto result = DataFilter::perform_ica (data, 2);
        // Component order and sign are arbitrary.
        std::cout << "Recovered " << std::get<3> (result).get_size (0) << " sources from "
                  << samples << " samples" << std::endl;
    }
    catch (const BrainFlowException &err)
    {
        std::cerr << err.what () << std::endl;
        return err.exit_code;
    }
    return 0;
}
