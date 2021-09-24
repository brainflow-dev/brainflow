// GanglionTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <Windows.h>

#include "../BA_Native_Interface/BA_Native.h"
#include "dll_loader.h"

void check_exit_code (int ec, const char *message)
{
    if (ec != (int)BrainAliveNativeLib::CustomExitCodesNative::STATUS_OK)
    {
        std::cerr <<  ((message + std::to_string(ec)).c_str()) << std::endl;
        exit (1);
    }
    else
    {
        std::cout << message << " done" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    
    DLLLoader *dll_loader = NULL;
    if (sizeof (void *) == 8)
    {
        const char *dll_name = "BA_Native_BLE.dll";
        dll_loader = new DLLLoader (dll_name);
    }
    else
    {
        const char *dll_name = "BA_Native_BLE.dll";
        dll_loader = new DLLLoader (dll_name);
    }
    if (!dll_loader->load_library ())
    {
        return 1;
    }
    DLLFunc func = dll_loader->get_address ("open_brainalive_mac_addr_native");
    if (func == NULL)
    {
        return 2;
    }
    int res = (func) (argv[1]);

   check_exit_code (res, "open");
    
    func = dll_loader->get_address ("start_stream_native");
    if (func == NULL)
    {
        return 3;
    }
    res = (func) (NULL);
    check_exit_code (res, "start");
    // in real usage it will be in abother thread in infinitive loop like while(!shouldStop).....
    struct BrainAliveNativeLib::BrainAliveDataNative data;
    func = dll_loader->get_address ("get_data_native");
    for (int i = 0; i < 500; i++)
    {
        res = (func) ((LPVOID)&data);
        if (res == BrainAliveNativeLib::CustomExitCodesNative::STATUS_OK)
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
    func = dll_loader->get_address ("close_brainalive_native");
    (func) (NULL);
    return 0;
}