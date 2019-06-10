// GanglionTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <Windows.h>

#include "../Wrapper/GanglionNativeInterface.h"

void check_exit_code (int ec, const char *message)
{
    if (ec != (int)GanglionLibNative::CustomExitCodesNative::STATUS_OK)
    {
        std::cerr <<  ((message + std::to_string(ec)).c_str()) << std::endl;
        exit (1);
    }
    else
    {
        std::cout << message << " done" << std::endl;
    }
}

int main()
{
    struct GanglionLibNative::GanglionDataNative data;

    int res = GanglionLibNative::open_ganglion_native ();
    check_exit_code (res, "open");
    res = GanglionLibNative::pair_ganglion_native ();
    check_exit_code (res, "pair");
    res = GanglionLibNative::start_stream_native ();
    check_exit_code (res, "start");
    // in real usage it will be in abother thread in infinitive loop like while(!shouldStop).....
    for (int i = 0; i < 500; i++)
    {
        res = GanglionLibNative::get_data_native (&data);
        if (res == GanglionLibNative::CustomExitCodesNative::STATUS_OK)
        {
            std::cout << "timestamp:" << data.timestamp << std::endl;
            for (int j = 0; j < 20; j++)
            {
                std::cout << (int) data.data[j] << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "no data in the queue" << std::endl;
            Sleep (100);
        }
    }
    res = GanglionLibNative::close_ganglion_native ();
    check_exit_code (res, "close");
    return 0;
}