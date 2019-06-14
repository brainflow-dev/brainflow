#ifndef RUNTIME_DLL_LOADER
#define RUNTIME_DLL_LOADER

#ifdef _WIN32
#include <windows.h>
// I dont want to care about unicode string, force undef it here to use char* as LPSTR
#undef _UNICODE
#undef UNICODE
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
        lib_instance = LoadLibrary (TEXT (this->dll_path));
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
    char dll_path[1024];
#ifdef _WIN32
    HINSTANCE lib_instance;
#endif
};
#endif
