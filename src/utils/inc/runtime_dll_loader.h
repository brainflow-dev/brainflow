#pragma once

#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <string>
#include <vector>
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
        strncpy (this->dll_path, dll_path, sizeof (this->dll_path) - 1);
        this->dll_path[sizeof (this->dll_path) - 1] = '\0';
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
            for (const std::string &candidate : get_dlopen_candidates ())
            {
                lib_instance = dlopen (candidate.c_str (), RTLD_LAZY | RTLD_DEEPBIND);
                if (lib_instance)
                {
                    return true;
                }
            }
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
#ifndef _WIN32
    std::vector<std::string> get_dlopen_candidates () const
    {
        std::vector<std::string> candidates;
        append_unique (candidates, this->dll_path);

#ifdef __APPLE__
        std::string original_path = this->dll_path;
        std::string directory = parent_directory (original_path);
        std::string file_name = last_path_component (original_path);
        std::string framework_name = apple_framework_name (file_name);

        if (!framework_name.empty ())
        {
            append_unique (candidates, framework_name + ".framework/" + framework_name);
            append_unique (candidates, "@rpath/" + framework_name + ".framework/" + framework_name);
            if (!directory.empty ())
            {
                append_unique (
                    candidates, directory + framework_name + ".framework/" + framework_name);
                std::string framework_parent = parent_frameworks_directory (directory);
                if (!framework_parent.empty ())
                {
                    append_unique (candidates,
                        framework_parent + framework_name + ".framework/" + framework_name);
                }
            }
        }
#endif
        return candidates;
    }

    static void append_unique (std::vector<std::string> &values, const std::string &value)
    {
        if (value.empty ())
        {
            return;
        }
        for (const std::string &existing : values)
        {
            if (existing == value)
            {
                return;
            }
        }
        values.push_back (value);
    }

#ifdef __APPLE__
    static std::string last_path_component (const std::string &path)
    {
        size_t slash = path.find_last_of ('/');
        if (slash == std::string::npos)
        {
            return path;
        }
        return path.substr (slash + 1);
    }

    static std::string parent_directory (const std::string &path)
    {
        size_t slash = path.find_last_of ('/');
        if (slash == std::string::npos)
        {
            return "";
        }
        return path.substr (0, slash + 1);
    }

    static std::string parent_frameworks_directory (const std::string &directory)
    {
        std::string normalized = directory;
        if (!normalized.empty () && normalized[normalized.size () - 1] == '/')
        {
            normalized.resize (normalized.size () - 1);
        }

        size_t framework_suffix = normalized.rfind (".framework");
        if (framework_suffix == std::string::npos)
        {
            return "";
        }

        size_t framework_dir_start = normalized.find_last_of ('/', framework_suffix);
        if (framework_dir_start == std::string::npos)
        {
            return "";
        }
        return normalized.substr (0, framework_dir_start + 1);
    }

    static std::string apple_framework_name (const std::string &file_name)
    {
        std::string name = file_name;
        const std::string dylib_suffix = ".dylib";
        if (name.size () > dylib_suffix.size () &&
            name.substr (name.size () - dylib_suffix.size ()) == dylib_suffix)
        {
            name.resize (name.size () - dylib_suffix.size ());
        }
        if (name.size () > 3 && name.substr (0, 3) == "lib")
        {
            name = name.substr (3);
        }
        return name;
    }
#endif
#endif

    char dll_path[1024];
#ifdef _WIN32
    HINSTANCE lib_instance;
#else
    void *lib_instance;
#endif
};
