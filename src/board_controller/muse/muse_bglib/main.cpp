#include <stdlib.h>

#include "muse_bglib_helper.h"
#include "muse_functions.h"


int initialize (void *param)
{
    return MuseBGLibHelper::get_instance ()->initialize (param);
}

int open_device (void *param)
{
    return MuseBGLibHelper::get_instance ()->open_device ();
}

int stop_stream (void *param)
{
    return MuseBGLibHelper::get_instance ()->stop_stream ();
}

int start_stream (void *param)
{
    return MuseBGLibHelper::get_instance ()->start_stream ();
}

int close_device (void *param)
{
    return MuseBGLibHelper::get_instance ()->close_device ();
}

int get_data (void *param)
{
    return MuseBGLibHelper::get_instance ()->get_data (param);
}

int release (void *param)
{
    return MuseBGLibHelper::get_instance ()->release ();
}

int config_device (void *param)
{
    return MuseBGLibHelper::get_instance ()->config_device (param);
}
