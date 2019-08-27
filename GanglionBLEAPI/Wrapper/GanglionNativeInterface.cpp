#include <Windows.h>
#include <atlstr.h>

#include "GanglionNativeInterface.h"
#include "Wrapper.h"

#define MAX_PATH 2048

using namespace GanglionLib;
using namespace Wrapper;
using namespace System;
using namespace System::IO;
using namespace System::Reflection;

namespace GanglionLibNative
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
    Assembly ^assembly_resolve (Object ^Sender, ResolveEventArgs ^args) {
        AssemblyName ^assemblyName = gcnew AssemblyName (args->Name);
        String ^target_file_name = gcnew String ("GanglionLib.dll");
        if (assemblyName->Name == "GanglionLib")
        {
            // try to search via GANGLION_LIB_PATH first
            try
            {
                String ^ganglion_lib_path =
                    Environment::GetEnvironmentVariable ("GANGLION_LIB_PATH");
                array<String ^> ^files = Directory::GetFiles (ganglion_lib_path);
                for (int j = 0; j < files->Length; j++)
                {
                    if (files[j]->EndsWith (target_file_name))
                    {
                        return Assembly::LoadFile (files[j]);
                    }
                }
            }
            catch (Exception ^ex)
            {
            }
            // try to search via Path next
            String ^path = Environment::GetEnvironmentVariable ("Path");
            auto folders = path->Split (';');
            for (int i = 0; i < folders->Length; i++)
            {
                try
                {
                    array<String ^> ^files = Directory::GetFiles (folders[i]);
                    for (int j = 0; j < files->Length; j++)
                    {
                        if (files[j]->EndsWith (target_file_name))
                        {
                            return Assembly::LoadFile (files[j]);
                        }
                    }
                }
                catch (Exception ^ex)
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
			DWORD ret = GetEnvironmentVariable (L"GANGLION_LIB_PATH", val, MAX_PATH);
			if (ret == 0)
			{
				// get path of this dll and use it as a hint for C# dll search via state var
				CStringW this_path = get_dll_path ();
				SetEnvironmentVariable (L"GANGLION_LIB_PATH", this_path);
			}

			AppDomain::CurrentDomain->AssemblyResolve +=
				gcnew ResolveEventHandler (assembly_resolve);
			is_initialized = true;
		}
		return (int)CustomExitCodesNative::STATUS_OK;
	}

    int open_ganglion_native (void *param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->open_ganglion ();
    }

    int open_ganglion_mac_addr_native (void *param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        String ^mac_new = gcnew String ((char *)param);
        return wrapper->open_ganglion (mac_new);
    }

    int close_ganglion_native (void *param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->close_ganglion ();
    }

    int start_stream_native (void *param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->start_stream ();
    }

    int stop_stream_native (void *param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->stop_stream ();
    }

    int get_data_native (void *param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        BoardData ^managedData = wrapper->get_data ();
        if (managedData->exit_code != (int)CustomExitCodes::STATUS_OK)
        {
            return managedData->exit_code;
        }
        struct GanglionDataNative *boardData = (struct GanglionDataNative *)param;
        boardData->timestamp = managedData->timestamp;
        for (int i = 0; i < managedData->data->Length; i++)
        {
            boardData->data[i] = (unsigned char)managedData->data[i];
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

	int config_board_native (void *param)
	{
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        String ^conf_str = gcnew String ((char *)param);
        return wrapper->config_board (conf_str, true); // use send characteristic
	}
}
