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
    int res = 0;
    try
    {
        double *window_data = NULL;
        int window_len = 20;
        for (int window_function = 0; window_function < 4; window_function++)
        {
            std::cout << "Current window function ID: " << window_function << std::endl;
            std::cout << "Window data :" << std::endl;

            // just apply different windowing algorithms and print results
            switch (window_function)
            {
                case 0:
                    window_data = DataFilter::get_window (window_function, window_len);
                    break;
                case 1:
                    window_data = DataFilter::get_window (window_function, window_len);
                    break;
                case 2:
                    window_data = DataFilter::get_window (window_function, window_len);
                    break;
                case 3:
                    window_data = DataFilter::get_window (window_function, window_len);
                    break;
            }
            std::cout << window_data << std::endl;
            delete[] window_data;
        }
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
    }

    return res;
}