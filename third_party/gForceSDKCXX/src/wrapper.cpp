#include <atomic>
#include <deque>
#include <thread>
#include <utility>
#include <windows.h>

#include "gforce.h"

#include "gforce_handle.h"
#include "gforce_wrapper_functions.h"

#include "brainflow_array.h"
#include "brainflow_input_params.h"
#include "spinlock.h"

#include "json.hpp"

using json = nlohmann::json;
using namespace gf;
using namespace std;

volatile int iExitCode = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
volatile bool bShouldStopStream = true;
volatile bool bShouldStopThread = true;
volatile bool bInitialized = false;
std::thread tReadThread; // thread for pooling
gfsPtr<Hub> pHub = NULL;
gfsPtr<GforceHandle> gforceHandle = NULL;
gfsPtr<HubListener> listener = NULL;
SpinLock spinLock;
std::deque<BrainFlowArray<double, 1>> dataQueue;

void threadFunc ()
{
    GF_UINT32 period = 1; // ms
    while (!bShouldStopThread)
    {
        GF_RET_CODE retCode = pHub->run (period);
    }
}

int initialize (void *param)
{
    if (bInitialized)
    {
        return (int)BrainFlowExitCodes::PORT_ALREADY_OPEN_ERROR;
    }
    iExitCode = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    pHub = HubManager::getHubInstance (_T("GForceBrainFlowWrapper"));
    pHub->setWorkMode (WorkMode::Polling); // means that need to run loop manually
    // create the listener implementation and register to hub
    std::tuple<int, struct BrainFlowInputParams, json> *info =
        (std::tuple<int, struct BrainFlowInputParams, json> *)param;
    gforceHandle = make_shared<GforceHandle> (pHub, std::get<0> (*info));
    listener = static_pointer_cast<HubListener> (gforceHandle);
    GF_RET_CODE retCode = pHub->registerListener (listener);
    if (retCode != GF_RET_CODE::GF_SUCCESS)
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    pHub->init ();
    // start scan and waiting for discovery
    retCode = pHub->startScan ();
    if (retCode != GF_RET_CODE::GF_SUCCESS)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    // run pooling thread
    bShouldStopThread = false;
    tReadThread = std::thread (threadFunc);
    // wait for initialization
    int numAttempts = 500;
    int curAttemp = 0;
    int sleepTime = 10;
    while ((curAttemp < numAttempts) && (iExitCode == (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR))
    {
        curAttemp++;
        Sleep (sleepTime);
    }

    if (iExitCode == (int)BrainFlowExitCodes::STATUS_OK)
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

int start_stream (void *param)
{
    if (!bInitialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    gforceHandle->logger->info ("starting acqusition.");
    bShouldStopStream = false;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int stop_stream (void *param)
{
    if (!bInitialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    gforceHandle->logger->info ("stop acqusition.");
    bShouldStopStream = true;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int release (void *param)
{
    if (!bInitialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    bShouldStopStream = true;
    bShouldStopThread = true;
    tReadThread.join ();
    pHub->unRegisterListener (listener);
    pHub->deinit ();
    bInitialized = false;
    spinLock.lock ();
    dataQueue.clear ();
    spinLock.unlock ();

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_data (void *param)
{
    if (!bInitialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    spinLock.lock ();

    if (dataQueue.empty ())
    {
        res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    else
    {
        try
        {
            double *board_data = (double *)param;
            BrainFlowArray<double, 1> data = dataQueue.at (0);
            for (int i = 0; i < data.get_size (0); i++)
            {
                board_data[i] = data[i];
            }
            dataQueue.pop_front ();
        }
        catch (...)
        {
            res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
        }
    }
    spinLock.unlock ();
    return res;
}

// stubs for dyn_lib_board class
int open_device (void *param)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int close_device (void *param)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int config_device (void *param)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
