#include <atomic>
#include <thread>

#include "gforce.h"

#include "gforce_handle.h"
#include "gforce_wrapper_functions.h"
#include "gforce_wrapper_types.h"

using namespace gf;
using namespace std;

volatile int iExitcode = (int)GforceWrapperExitCodes::SYNC_ERROR;
volatile bool bShouldStopStream = true;
volatile bool bInitialized = false;
std::thread tReadThread;
gfsPtr<Hub> pHub = HubManager::getHubInstance (_T("GForceBrainFlowWrapper"));

int gforce_initialize (void *param)
{
    if (bInitialized)
    {
        return (int)GforceWrapperExitCodes::ALREADY_INITIALIZED;
    }
    pHub->init ();
    GF_UINT32 period = 10; // ms, timeout which I dont understand
    GF_RET_CODE retCode = pHub->run (period);
    if (retCode != GF_RET_CODE::GF_SUCCESS)
    {
        return (int)GforceWrapperExitCodes::HUB_INIT_FAILED;
    }
    bInitialized = true;

    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforce_start_streaming (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforce_stop_streaming (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforce_release (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforce_get_data (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    return (int)GforceWrapperExitCodes::STATUS_OK;
}
