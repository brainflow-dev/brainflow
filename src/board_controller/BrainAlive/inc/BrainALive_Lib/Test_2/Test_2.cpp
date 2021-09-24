
#include <string>
#include <Windows.h>
#include <iostream>
#include "get_dll_dir.h"
#include "../BA_Native_Interface/BA_Native.h"

#include "load_dll.h"

void check_exit_code (int ec, const char *message)
{
    if (ec != (int)BrainAliveNativeLib::CustomExitCodesNative::STATUS_OK)
    {
        std::cerr << ((message + std::to_string (ec)).c_str ()) << std::endl;
        exit (1);
    }
    else
    {
        std::cout << message << " done" << std::endl;
    }
}

int main (int argc, char *argv[])
{
    char *argv_1[10] = {NULL};

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
        printf ("Library load fail");
        return 1;
    }
    else
    {
        printf ("\n Library loaded sucessfully");
    }
    int (*func_open_device) (void *) = (int (*) (void *))dll_loader->get_address ("open_brainalive_mac_addr_native");
    if (func_open_device == NULL)
    {
        printf ("function addr not find");
        return 1;
    }
    printf ("\nbefore connect");
    int res = (func_open_device) (argv_1[0]);
    printf ("\nafter connect");
    //check_exit_code (res, "\nopen");

    int (*func_start_device) (void *) =
        (int (*)(void *))dll_loader->get_address ("start_stream_native");
    if (func_start_device == NULL)
    {
        printf ("Start function addr not find");
        return 3;
    }
    printf ("\nbefore start");
    res = (func_start_device)(NULL);
    printf ("\after start");
    check_exit_code (res, "start");
    //// in real usage it will be in abother thread in infinitive loop like while(!shouldStop).....
    struct BrainAliveNativeLib::BrainAliveDataNative data;
    int (*func_get_data) (void *) =
        (int (*) (void *))dll_loader->get_address ("get_data_native");
    for (int i = 0; i < 500; i++)
    {
        res = (func_get_data) ((LPVOID)&data);
        if (res == BrainAliveNativeLib::CustomExitCodesNative::STATUS_OK)
        {
            std::cout << "timestamp:" << data.timestamp << std::endl;
            for (int j = 0; j < 20; j++)
            {
                std::cout << (int)data.data[j] << " ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "no data in the queue" << std::endl;
            Sleep (100);
        }
    }
    int (*func_close_data) (void *) =
        (int (*) (void *))dll_loader->get_address ("stop_stream_native");
    (func_close_data) (NULL);
    return 0;
}