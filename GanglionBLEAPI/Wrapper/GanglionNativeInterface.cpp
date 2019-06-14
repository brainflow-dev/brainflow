#include "GanglionNativeInterface.h"
#include "Wrapper.h"

using namespace GanglionLib;
using namespace Wrapper;

namespace GanglionLibNative
{
    int open_ganglion_native (LPVOID param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        return wrapper->open_ganglion ();
    }

    int open_ganglion_mac_addr_native (LPVOID param)
    {
        Ganglion ^wrapper = GanglionLibWrapper::instance->ganglion_obj;
        String ^macNew = gcnew String ((char*)param);
        return wrapper->open_ganglion (macNew);
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
