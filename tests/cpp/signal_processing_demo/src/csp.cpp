#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"
#include "data_filter.h"

using namespace std;

int main (int argc, char *argv[])
{
    int n_ep = 2;
    int n_ch = 2;
    int n_times = 4;

    double labels[3] = {0, 1};
    double ar[2][2][4] = {{{6, 3, 1, 5}, {3, 0, 5, 1}}, {{1, 5, 6, 2}, {5, 1, 2, 2}}};

    double ***data = new double **[n_ep];

    // Init data array
    for (int e = 0; e < n_ep; e++)
    {
        data[e] = new double *[n_ch];
        std::cout << "data = " << std::endl;
        for (int c = 0; c < n_ch; c++)
        {
            data[e][c] = new double[n_times];
            for (int t = 0; t < n_times; t++)
            {
                data[e][c][t] = ar[e][c][t];
                std::cout << data[e][c][t] << "  ";
            }
            std::cout << std::endl;
        }
    }

    // Get CSP filters
    std::pair<double **, double *> output = DataFilter::get_csp (data, labels, n_ep, n_ch, n_times);

    // Print results
    std::cout << "\nfilters = ";
    for (int i = 0; i < n_ch; i++)
    {
        std::cout << "\n";
        for (int j = 0; j < n_ch; j++)
        {
            std::cout << output.first[i][j] << "  ";
        }
    }
    std::cout << "\n";

    // Release allocated memory
    for (int e = 0; e < n_ep; e++)
    {
        for (int c = 0; c < n_ch; c++)
        {
            delete[] data[e][c];
        }
        delete[] data[e];
    }

    for (int c = 0; c < n_ch; c++)
    {
        delete[] output.first[c];
    }

    delete[] data;
    delete[] output.first;
    delete[] output.second;
}
