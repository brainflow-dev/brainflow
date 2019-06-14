#ifndef RUNTIME_DLL_LOADER
#define RUNTIME_DLL_LOADER

#include <string.h>
#ifdef _WIN32
#include <windows.h>
typedef __declspec(dllimport) int(__cdecl *DLLFunc) (LPVOID);
#endif


class DLLLoader
{
public:
    DLLLoader (const char *dll_path)
    {
        strcpy (this->dll_path, dll_path);
        lib_instance = NULL;
    }

    ~DLLLoader ()
    {
        free_library ();
    }

    // WINDOWS PART
#ifdef _WIN32
    bool load_library ()
    {
        lib_instance = LoadLibrary (this->dll_path);
        if (lib_instance == NULL)
        {
            return false;
        }
        return true;
    }

    DLLFunc get_address (const char *function_name)
    {
        if (this->lib_instance == NULL)
        {
            return NULL;
        }
        return (DLLFunc)GetProcAddress (this->lib_instance, function_name);
    }

    void free_library ()
    {
        if (this->lib_instance)
        {
            FreeLibrary (this->lib_instance);
        }
    }

    // UNIX && APPLE PART
#else

#endif

private:
#ifdef _WIN32
    char dll_path[64];
    HINSTANCE lib_instance;
#endif
};
#endif
