#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <atlstr.h>
#include <string.h>

// no unicode!
inline bool get_dll_path (char *res)
{
    CString this_path = "";
    WCHAR path[MAX_PATH];
    HMODULE hm;
    if (GetModuleHandleExW (
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPWSTR)&get_dll_path, &hm))
    {
        GetModuleFileNameW (hm, path, sizeof (path));
        PathRemoveFileSpecW (path);
        this_path = CStringW (path);
        if (!this_path.IsEmpty () && this_path.GetAt (this_path.GetLength () - 1) != '\\')
            this_path += "\\";
    }
    else
    {
        return false;
    }
    char *buf = NULL;
    buf = this_path.GetBuffer (this_path.GetLength ());
    this_path.ReleaseBuffer ();
    strcpy (res, buf);
    return true;
}

#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <string.h>
#include <string>


inline bool get_dll_path (char *res)
{
    Dl_info dl_info;
    if (dladdr ((void *)get_dll_path, &dl_info))
    {
        std::string full_path = dl_info.dli_fname;
        std::string dir_path = full_path.substr (0, full_path.find_last_of ("/") + 1);
        strcpy (res, dir_path.c_str ());
        return true;
    }
    return false;
}

#endif
