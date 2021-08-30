#include <Windows.h>
#include <atlstr.h>

#include "../BA_Native_Interface/BA_Native.h"
#include "Wrapper.h"

#define MAX_PATH 2048

using namespace BrainAlive_Lib;
using namespace Wrapper;
using namespace System;
using namespace System::IO;
using namespace System::Reflection;

namespace BrainAliveNativeLib
{
    CStringW get_dll_path ()
    {
        CStringW this_path = L"";
        WCHAR path[MAX_PATH];
        HMODULE hm;
        if (GetModuleHandleExW (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPWSTR)&get_dll_path, &hm))
        {
            GetModuleFileNameW (hm, path, sizeof (path));
            PathRemoveFileSpecW (path);
            this_path = CStringW (path);
            if (!this_path.IsEmpty () && this_path.GetAt (this_path.GetLength () - 1) != '\\')
                this_path += L"\\";
        }
        else
        {
            return L"";
        }

        return this_path;
    }

    bool is_initialized = false;
    Assembly ^
        assembly_resolve (Object ^ Sender, ResolveEventArgs ^ args)
    {
        AssemblyName ^ assemblyName = gcnew AssemblyName (args->Name);
        String ^ target_file_name = gcnew String ("BA_BLE_Lib.dll");
        if (assemblyName->Name == "BA_BLE_Lib")
        {
            // try to search via BRAINALIVE_LIB_PATH first
            try
            {
                String ^ brainalive_lib_path =
                    Environment::GetEnvironmentVariable ("BRAINALIVE_LIB_PATH");
                array<String ^> ^ files = Directory::GetFiles (brainalive_lib_path);
                for (int j = 0; j < files->Length; j++)
                {
                    if (files[j]->EndsWith (target_file_name))
                    {
                        return Assembly::LoadFile (files[j]);
                    }
                }
            }
            catch (Exception ^ ex)
            {
            }
            // try to search via Path next
            String ^ path = Environment::GetEnvironmentVariable ("Path");
            auto folders = path->Split (';');
            for (int i = 0; i < folders->Length; i++)
            {
                try
                {
                    array<String ^> ^ files = Directory::GetFiles (folders[i]);
                    for (int j = 0; j < files->Length; j++)
                    {
                        if (files[j]->EndsWith (target_file_name))
                        {
                            return Assembly::LoadFile (files[j]);
                        }
                    }
                }
                catch (Exception ^ ex)
                {
                }
            }
        }
        return nullptr;
    }

        int initialize_native (void *param)
    {
        if (!is_initialized)
        {
            TCHAR tmp[MAX_PATH];
            LPTSTR val = (LPTSTR)tmp;
            DWORD ret = GetEnvironmentVariable (L"BRAINALIVE_LIB_PATH", val, MAX_PATH);
            if (ret == 0)
            {
                // get path of this dll and use it as a hint for C# dll search via state var
                CStringW this_path = get_dll_path ();
                SetEnvironmentVariable (L"BRAINALIVE_LIB_PATH", this_path);
            }

            AppDomain::CurrentDomain->AssemblyResolve +=
                gcnew ResolveEventHandler (assembly_resolve);
            is_initialized = true;
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int open_brainalive_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        return wrapper->open_brainalive ();
    }

    int open_brainalive_mac_addr_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        String ^ mac_new = gcnew String ((char *)param);
        return wrapper->open_brainalive (mac_new);
    }

    int close_brainalive_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        return wrapper-> close_brainalive ();
    }

    int start_stream_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        return wrapper->start_stream ();
    }

    int stop_stream_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        return wrapper->stop_stream ();
    }

    int get_data_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        BoardData ^ managed_data = wrapper->get_data ();
        if (managed_data->exit_code != (int)CustomExitCodes::STATUS_OK)
        {
            return managed_data->exit_code;
        }
        struct BrainAliveDataNative *board_data = (struct BrainAliveDataNative *)param;
        board_data->timestamp = managed_data->timestamp;
        for (int i = 0; i < managed_data->data->Length; i++)
        {
            board_data->data[i] = (unsigned char)managed_data->data[i];
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int config_board_native (void *param)
    {
        BrainAlive ^ wrapper = BrainAliveLibWrapper::instance->brainalive_obj;
        String ^ conf_str = gcnew String ((char *)param);
        return wrapper->config_board (conf_str, true); // use send characteristic
    }

    int release_native (void *param)
    {
        return (int)CustomExitCodesNative::STATUS_OK;
    }
}