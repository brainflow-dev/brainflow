
#include <string>
#include <Windows.h>
#include <iostream>
#include "../BA_Native_Interface/BA_Native.h"
#include "load_dll.h"
#include "get_dll_dir.h"

int main (int argc, char *argv[])
{

    //std::cout << "Hello World!\n";
    DLLLoader *dll_loader = NULL;

    //char Brainalivelib_dir[1024];
    //bool res = get_dll_path (Brainalivelib_dir);
    //std::string brainalivelib_path = "";
    //std::string lib_name;

    //if (sizeof (void *) == 8)
    //{
    //      lib_name = "BA_Native_BLE.dll";
    //}
    //else
    //{
    //    lib_name = "BA_Native_BLE.dll";
    //}
    //if (res)
    //{
    //    brainalivelib_path = std::string (Brainalivelib_dir) + lib_name;
    //}
    //else
    //{
    //    brainalivelib_path = lib_name;
    //}

    // dll_loader = new DLLLoader (brainalivelib_path.c_str ());

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
        printf ("Library loaded sucessfully");
        //return 0;
    }
    else
    {

        printf ("Library load fail");
        return 1;
    }

       DLLFunc func_open_device = (dll_loader->get_address ("open_brainalive_mac_addr_native"));

        if (func_open_device == NULL)
        {
            printf("\n failed to find open brainalive funtion address");
            return 1;
        }
         printf("\nbefore connect");
        int res_1 = func_open_device (argv[1]);
         printf ("\nafter connect");
  
         return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
