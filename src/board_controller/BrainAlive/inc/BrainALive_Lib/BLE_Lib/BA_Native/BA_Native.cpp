#include <Windows.h>
#include <atlstr.h>

#include "BA_Native.h"
#include "Wrapper.h"

using namespace BLE_Lib;
using namespace Wrapper;
using namespace System;
using namespace System::IO;
using namespace System::Reflection;

namespace BrainAliveNativeLib
{
    CStringW get_dll_path()
    {
        CStringW this_path = L"";
        WCHAR path[MAX_PATH];
        HMODULE hm;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPWSTR)&get_dll_path, &hm))
        {
            GetModuleFileNameW(hm, path, sizeof(path));
            PathRemoveFileSpecW(path);
            this_path = CStringW(path);
            if (!this_path.IsEmpty() && this_path.GetAt(this_path.GetLength() - 1) != '\\')
                this_path += L"\\";
        }
        else
        {
            return L"";
        }

        return this_path;
    }

    bool is_initialized = false;
    Assembly^
        assembly_resolve(Object^ Sender, ResolveEventArgs^ args)
    {
        AssemblyName^ assemblyName = gcnew AssemblyName(args->Name);
        String^ target_file_name = gcnew String("BLE_Lib.dll");
        if (assemblyName->Name == "BLE_Lib")
        {
            // try to search via BRAINALIVE_LIB_PATH first
            try
            {
                String^ brainalive_lib_path =
                    Environment::GetEnvironmentVariable("BRAINALIVE_LIB_PATH");
                array<String^>^ files = Directory::GetFiles(brainalive_lib_path);
                for (int j = 0; j < files->Length; j++)
                {
                    if (files[j]->EndsWith(target_file_name))
                    {
                        return Assembly::LoadFile(files[j]);
                    }
                }
            }
            catch (Exception^ ex)
            {
            }
            // try to search via Path next
            String^ path = Environment::GetEnvironmentVariable("Path");
            auto folders = path->Split(';');
            for (int i = 0; i < folders->Length; i++)
            {
                try
                {
                    array<String^>^ files = Directory::GetFiles(folders[i]);
                    for (int j = 0; j < files->Length; j++)
                    {
                        if (files[j]->EndsWith(target_file_name))
                        {
                            return Assembly::LoadFile(files[j]);
                        }
                    }
                }
                catch (Exception^ ex)
                {
                }
            }
        }
        return nullptr;
    }

    int initialize_native(void* param)
    {
        if (!is_initialized)
        {
            TCHAR tmp[MAX_PATH];
            LPTSTR val = (LPTSTR)tmp;
            DWORD ret = GetEnvironmentVariable(L"BRAINALIVE_LIB_PATH", val, MAX_PATH);
            if (ret == 0)
            {
                // get path of this dll and use it as a hint for C# dll search via state var
                CStringW this_path = get_dll_path();
                SetEnvironmentVariable(L"BRAINALIVE_LIB_PATH", this_path);
            }

            AppDomain::CurrentDomain->AssemblyResolve +=
                gcnew ResolveEventHandler(assembly_resolve);
            is_initialized = true;
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int open_brainalive_native(wchar_t* param)
    {
        Brainalive^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        return wrapper->Scan_BLE(param);
    }
}