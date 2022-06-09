#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "data_filter.h"


int main (int argc, char *argv[])
{
    constexpr int n_ep = 2;
    constexpr int n_ch = 2;
    constexpr int n_times = 4;
    constexpr int total_len = n_ep * n_ch * n_times;

    double labels_array[n_ep] = {0, 1};
    double data_array[total_len] = {6, 3, 1, 5, 3, 0, 5, 1, 1, 5, 6, 2, 5, 1, 2, 2};

    BrainFlowArray<double, 1> labels (labels_array, n_ep);
    BrainFlowArray<double, 3> data (data_array, n_ep, n_ch, n_times);
    std::cout << data << std::endl;

    std::pair<BrainFlowArray<double, 2>, BrainFlowArray<double, 1>> output =
        DataFilter::get_csp (data, labels);

    std::cout << output.first << std::endl;
    std::cout << output.second << std::endl;
}
