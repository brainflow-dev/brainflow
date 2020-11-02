#include <atomic>
#include <thread>
#include <windows.h>

#include "gforce.h"

#include "gforce_handle.h"
#include "gforce_wrapper_functions.h"
#include "gforce_wrapper_types.h"

using namespace gf;
using namespace std;

volatile int iExitCode = (int)GforceWrapperExitCodes::SYNC_ERROR;
volatile bool bShouldStopStream = true;
volatile bool bShouldStopThread = true;
volatile bool bInitialized = false;
std::thread tReadThread; // thread for pooling
gfsPtr<Hub> pHub = NULL;
gfsPtr<HubListener> listener = NULL;

void threadFunc ()
{
    GF_UINT32 period = 20; // ms
    while (!bShouldStopThread)
    {
        GF_RET_CODE retCode = pHub->run (period);
    }
}

int gforceInitialize (void *param)
{
    if (bInitialized)
    {
        return (int)GforceWrapperExitCodes::ALREADY_INITIALIZED;
    }
    iExitCode = (int)GforceWrapperExitCodes::SYNC_ERROR;
    pHub = HubManager::getHubInstance (_T("GForceBrainFlowWrapper"));
    pHub->setWorkMode (WorkMode::Polling); // means that need to run loop manually
    // create the listener implementation and register to hub
    gfsPtr<GForceHandle> gforceHandle = make_shared<GForceHandle> (pHub);
    gfsPtr<HubListener> listener = static_pointer_cast<HubListener> (gforceHandle);
    GF_RET_CODE retCode = pHub->registerListener (listener);
    if (retCode != GF_RET_CODE::GF_SUCCESS)
    {
        return (int)GforceWrapperExitCodes::HUB_INIT_FAILED;
    }
    pHub->init ();
    // start scan and waiting for discovery
    retCode = pHub->startScan ();
    if (retCode != GF_RET_CODE::GF_SUCCESS)
    {
        return (int)GforceWrapperExitCodes::SCAN_INIT_FAILED;
    }
    // run pooling thread
    bShouldStopThread = false;
    tReadThread = std::thread (threadFunc);
    // wait for initialization
    int numAttempts = 500;
    int curAttemp = 0;
    int sleepTime = 10;
    while ((curAttemp < numAttempts) && (iExitCode == (int)GforceWrapperExitCodes::SYNC_ERROR))
    {
        curAttemp++;
        Sleep (sleepTime);
    }

    if (iExitCode == (int)GforceWrapperExitCodes::STATUS_OK)
    {
        bInitialized = true;
    }
    else
    {
        bShouldStopThread = true;
        tReadThread.join ();
        pHub->unRegisterListener (listener);
        pHub->deinit ();
    }
    return iExitCode;
}

int gforceStartStreaming (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    bShouldStopStream = false;

    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforceStopStreaming (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    bShouldStopStream = true;

    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforceRelease (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    bShouldStopStream = true;
    bShouldStopThread = true;
    tReadThread.join ();
    pHub->unRegisterListener (listener);
    pHub->deinit ();

    return (int)GforceWrapperExitCodes::STATUS_OK;
}

int gforceGetData (void *param)
{
    if (!bInitialized)
    {
        return (int)GforceWrapperExitCodes::NOT_INITIALIZED;
    }
    return (int)GforceWrapperExitCodes::STATUS_OK;
}
