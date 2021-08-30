// GanglionTest.cpp : This file contains the 'main' function. Program execution begins and ends
// there.
//

#include <Windows.h>
#include <iostream>
#include <string>

#include "../BA_Native/BA_Native.h"
#include "dll_loader.h"

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
    DLLLoader *dll_loader = NULL;
    
    if (sizeof (void *) == 8)
    {
        const char *dll_name = "BA_Native.dll";
        dll_loader = new DLLLoader (dll_name);
    }
    else
    {
        const char *dll_name = "BA_Native.dll";
        dll_loader = new DLLLoader (dll_name);
    }
    if (!dll_loader->load_library ())
    {
        return 1;
    }
    DLLFunc func = dll_loader->get_address ("open_brainalive_native");
    if (func == NULL)
    {
        return 2;
    }
    int res = (func)(argv);
    for (int i=0;i<5;i++)
    printf (argv[i]);
    return 0;
}