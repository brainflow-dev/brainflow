#pragma once

#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


// for macos and maybe ancient versions of glibc
#ifndef RTLD_DEEPBIND
#define RTLD_DEEPBIND 0
#endif


class DLLLoader
{
public:
    DLLLoader (const char *dll_path)
    {
        strcpy (this->dll_path, dll_path);
        this->lib_instance = NULL;
    }

    ~DLLLoader ()
    {
        free_library ();
    }

    // WINDOWS PART
#ifdef _WIN32
    bool load_library ()
    {
        if (this->lib_instance == NULL)
        {
            this->lib_instance = LoadLibrary (this->dll_path);
            if (this->lib_instance == NULL)
            {
                return false;
            }
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
            this->lib_instance = NULL;
        }
    }
    // linux part
#else
    bool load_library ()
    {
        if (this->lib_instance == NULL)
        {
            // RTLD_DEEPBIND will search for symbols in loaded lib first and after that in global
            // scope
            lib_instance = dlopen (this->dll_path, RTLD_LAZY | RTLD_DEEPBIND);
            if (!lib_instance)
            {
                return false;
            }
        }
        return true;
    }

    void *get_address (const char *function_name)
    {
        if (this->lib_instance == NULL)
        {
            return NULL;
        }
        return dlsym (this->lib_instance, function_name);
    }

    void free_library ()
    {
        if (this->lib_instance)
        {
            dlclose (this->lib_instance);
            this->lib_instance = NULL;
        }
    }
#endif

private:
    char dll_path[1024];
#ifdef _WIN32
    HINSTANCE lib_instance;
#else
    void *lib_instance;
#endif
};
