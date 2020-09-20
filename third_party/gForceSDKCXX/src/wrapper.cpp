#include <atomic>

#include "gforce.h"

#include "gforce_wrapper_function.h"
#include "gforce_wrapper_types.h"
#include "thread_safe_queue.h"

using namespace gf;
using namespace std;

volatile int iExitcode = (int)CustomExitCodes::SYNC_ERROR;
volatile bool bShouldStopStream = true;
volatile bool bInitialized = false;
std::thread tReadThread;

int gforce_initialize (void *param)
{
    if (initialized)
    {
        return (int)CustomExitCodes::ALREADY_INITIALIZED;
    }
    hub.init ();
    bool ret = hub.run ();
    if (!ret)
    {
        return (int)CustomExitCodes::HUB_INIT_FAILED;
    }
    initialized.store (true);

    return (int)CustomExitCodes::STATUS_OK;
}

int gforce_start_streaming (void *param)
{
    if (!initialized)
    {
        return (int)CustomExitCodes::NOT_INITIALIZED;
    }
    return (int)CustomExitCodes::STATUS_OK;
}

int gforce_stop_streaming (void *param)
{
    if (!initialized)
    {
        return (int)CustomExitCodes::NOT_INITIALIZED;
    }
    return (int)CustomExitCodes::STATUS_OK;
}

int gforce_release (void *param)
{
    if (!initialized)
    {
        return (int)CustomExitCodes::NOT_INITIALIZED;
    }
    return (int)CustomExitCodes::STATUS_OK;
}

int gforce_get_data (void *param)
{
    if (!initialized)
    {
        return (int)CustomExitCodes::NOT_INITIALIZED;
    }
    return (int)CustomExitCodes::STATUS_OK;
}
