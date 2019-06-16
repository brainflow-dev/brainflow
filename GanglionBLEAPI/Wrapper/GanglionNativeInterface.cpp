#include "GanglionNativeInterface.h"
#include "Wrapper.h"

using namespace GanglionLib;
using namespace Wrapper;
using namespace System;
using namespace System::IO;
using namespace System::Reflection;

namespace GanglionLibNative
{
    bool is_initialized = false;
    Assembly ^assembly_resolve (Object ^Sender, ResolveEventArgs ^args)
    {
        AssemblyName ^assemblyName = gcnew AssemblyName (args->Name);
        if (assemblyName->Name == "GanglionLib")
        {
            String ^target_file_name = gcnew String ("GanglionLib.dll");
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

    int initialize (LPVOID param)
    {
        if (!is_initialized)
        {
            AppDomain::CurrentDomain->AssemblyResolve +=
                gcnew ResolveEventHandler (assembly_resolve);
            is_initialized = true;
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int open_ganglion_native (LPVOID param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->open_ganglion ();
    }

    int open_ganglion_mac_addr_native (LPVOID param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        String ^mac_new = gcnew String ((char*)param);
        return wrapper->open_ganglion (mac_new);
    }

    int close_ganglion_native (LPVOID param)
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->close_ganglion ();
    }

    int start_stream_native (LPVOID param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->start_stream ();
    }

    int stop_stream_native (LPVOID param)
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->stop_stream ();
    }

    int get_data_native (LPVOID param)
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
            boardData->data[i] = (unsigned char) managedData->data[i];
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }
}
