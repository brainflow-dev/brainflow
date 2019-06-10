#include "GanglionNativeInterface.h"
#include "Wrapper.h"

using namespace GanglionLib;
using namespace Wrapper;

namespace GanglionLibNative
{
    int open_ganglion_native ()
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->open_ganglion ();
    }

    int open_ganglion_mac_addr_native (char *macAddr)
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        String ^macNew = gcnew String (macAddr);
        return wrapper->open_ganglion (macNew);
    }

    int pair_ganglion_native ()
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->pair_ganglion ();
    }

    int close_ganglion_native ()
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->close_ganglion ();
    }

    int start_stream_native ()
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->start_stream ();
    }

    int stop_stream_native ()
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->stop_stream ();
    }

    int get_data_native (struct GanglionDataNative *boardData)
    {
        Ganglion ^ wrapper = GanglionLibWrapper::instance->ganglion_obj;
        BoardData ^managedData = wrapper->get_data ();
        if (managedData->exit_code != (int)CustomExitCodes::STATUS_OK)
        {
            return managedData->exit_code;
        }
        boardData->timestamp = managedData->timestamp;
        for (int i = 0; i < managedData->data->Length; i++)
        {
            boardData->data[i] = (unsigned char) managedData->data[i];
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }
}