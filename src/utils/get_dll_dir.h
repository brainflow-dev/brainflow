#ifndef GET_DLL_DIR
#define GET_DLL_DIR

#ifdef _WIN32
#include <Windows.h>
#include <atlstr.h>
#include <string.h>

// no unicode!
bool get_dll_path (char *res)
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
#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>

// untested
bool get_dll_path (char *res)
{
    Dl_info dl_info;
    if (dladdr ((void *)get_dll_path, &dl_info))
    {
        strcpy (res, dl_info.dli_fname);
        return true;
    }
    return false;
}

#endif
#endif
