#pragma once

#include <string.h>
#include <windows.h>



class DLLLoader
{
public:
    DLLLoader (const char *dll_path)
    {
        strcpy_s (this->dll_path, dll_path);
        lib_instance = NULL;
    }

    ~DLLLoader ()
    {
        free_library ();
    }

    bool load_library ()
    {
        lib_instance = LoadLibrary (this->dll_path);
        if (lib_instance == NULL)
        {
            return false;
        }
        return true;
    }

    void *get_address (const char *function_name)
    {
        if (this->lib_instance == NULL)
        {
            return NULL;
        }
        return (void *)GetProcAddress (this->lib_instance, function_name);
    }

    void free_library ()
    {
        if (this->lib_instance)
        {
            FreeLibrary (this->lib_instance);
        }
    }

private:
    char dll_path[64];
    HINSTANCE lib_instance;
};
