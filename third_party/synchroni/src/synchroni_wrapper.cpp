#if defined __APPLE__
#include <sensor/SensorController.hpp>
#else
#include "SensorController.hpp"
#endif

#include "board_info_getter.h"
#include "brainflow_boards.h"
#include "brainflow_input_params.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"
#include "synchroni_wrapper.h"
#include "timestamp.h"
#include <iostream>
#include <map>
#include <memory>
// #define ENABLE_LOGGER

using namespace std;
using namespace sensor;

const int PACKAGE_COUNT = 1;

class SynchroniBoardWrapper : public sensor::SensorProfileDelegate,
                              public enable_shared_from_this<SynchroniBoardWrapper>
{
public:
    std::shared_ptr<sensor::SensorProfile> profile;

    int board_id;
    struct BrainFlowInputParams params;
    json board_descr;

    deque<SensorData> lastEEG;
    deque<SensorData> lastECG;
    mutex dataMutex;

    DeviceInfo deviceInfo;
    double startStreamTimeStamp;

    SynchroniBoardWrapper ()
    {
    }

    virtual ~SynchroniBoardWrapper ()
    {
        profile = NULL;
    }

    virtual void onErrorCallback (
        std::shared_ptr<sensor::SensorProfile> profile, std::string errorMsg)
    {
    }

    virtual void onStateChange (
        std::shared_ptr<sensor::SensorProfile> profile, sensor::BLEDevice::State newState)
    {

        // throw BrainFlowException ("failed to get board info", res);
    }

    virtual void onSensorNotifyData (
        std::shared_ptr<sensor::SensorProfile> profile, const sensor::SensorData &rawData)
    {
        if (profile != this->profile)
        {
            return;
        }
        if (rawData.dataType == sensor::SensorData::NTF_EEG)
        {
            dataMutex.lock ();
            lastEEG.push_back (rawData);
            dataMutex.unlock ();
        }
        if (rawData.dataType == sensor::SensorData::NTF_ECG)
        {
            dataMutex.lock ();
            lastECG.push_back (rawData);
            dataMutex.unlock ();
        }
    }
};


class SynchroniControllerDelegate : public sensor::SensorControllerDelegate,
                                    public enable_shared_from_this<SynchroniControllerDelegate>
{
public:
    SynchroniControllerDelegate ()
    {

#ifdef ENABLE_LOGGER
        logger = spdlog::stderr_logger_mt ("SynchroniLogger");
        logger->set_level (spdlog::level::level_enum (0));
        logger->flush_on (spdlog::level::level_enum (0));
#else
        logger = spdlog::create<spdlog::sinks::null_sink_st> ("SynchroniLogger");
#endif
    }
    virtual ~SynchroniControllerDelegate ()
    {
    }

    map<string, std::shared_ptr<SynchroniBoardWrapper>> boardMap;
    std::mutex boardMapMutex;
    std::shared_ptr<spdlog::logger> logger;

    static std::shared_ptr<SynchroniControllerDelegate> sensorControllerDelegate;

    virtual void onSensorControllerEnableChanged (bool enabled)
    {
    }

    virtual void onSensorScanResult (vector<BLEDevice> bleDevices)
    {
    }
};

std::shared_ptr<SynchroniControllerDelegate> SynchroniControllerDelegate::sensorControllerDelegate;

std::once_flag initFlag;

////////////////////////////////////////


int synchroni_initialize (void *param)
{
    std::call_once (initFlag,
        [] ()
        {
            SynchroniControllerDelegate::sensorControllerDelegate =
                make_shared<SynchroniControllerDelegate> ();
            SensorController::getInstance ()->setDelegate (
                SynchroniControllerDelegate::sensorControllerDelegate);
        });

    std::tuple<int, struct BrainFlowInputParams, json> *info =
        (std::tuple<int, struct BrainFlowInputParams, json> *)(param);

    int board_id = std::get<0> (*info);
    struct BrainFlowInputParams params = std::get<1> (*info);
    json board_descr = std::get<2> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[params.mac_address] != NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }
    auto wrapper = make_shared<SynchroniBoardWrapper> ();
    wrapper->params = params;
    wrapper->board_id = board_id;
    wrapper->board_descr = board_descr;
    SynchroniControllerDelegate::sensorControllerDelegate->boardMap[params.mac_address] = wrapper;
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();

    return 0;
}
int synchroni_open_device (void *param)
{
    std::tuple<std::string> *info = (std::tuple<std::string> *)(param);
    std::string mac_address = std::get<0> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address] == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    auto wrapper = SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address];
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    int timeout = wrapper->params.timeout;
    if (timeout < 1)
    {
        timeout = 5;
    }
    if (wrapper->profile == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->logger->log (
            spdlog::level::info, "Use timeout for discovery: {}", timeout);
        auto controller = sensor::SensorController::getInstance ();
        if (!controller->isScaning ())
        {
            controller->startScan (2 * 1000);
        }

        auto profile = controller->getSensor (mac_address);

        for (int index = 0; index < timeout; ++index)
        {
            profile = controller->getSensor (mac_address);
            if (profile == NULL)
            {
#ifdef _WIN32
                Sleep (1000);
#else
                sleep (1);
#endif
            }
            else
            {
                break;
            }
        }
        controller->stopScan ();

        if (profile == NULL)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }

        wrapper->profile = profile;
        profile->setDelegate (wrapper);
    }

    if (wrapper->profile->getDeviceState () != BLEDevice::State::Ready)
    {
        wrapper->profile->connect ();

        for (int index = 0; index < timeout; ++index)
        {
            if (wrapper->profile->getDeviceState () != BLEDevice::State::Ready)
            {
#ifdef _WIN32
                Sleep (1000);
#else
                sleep (1);
#endif
            }
            else
            {
                break;
            }
        }

        if (wrapper->profile->getDeviceState () != BLEDevice::State::Ready)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }

    if (!wrapper->profile->hasInit ())
    {
        wrapper->profile->init (PACKAGE_COUNT, timeout * 1000,
            [] (bool succeed, string err)
            {
                if (err != "")
                {
                    SynchroniControllerDelegate::sensorControllerDelegate->logger->log (
                        spdlog::level::err, "init error: " + err);
                }
            });

        for (int index = 0; index < timeout; ++index)
        {
            if (!wrapper->profile->hasInit ())
            {
#ifdef _WIN32
                Sleep (1000);
#else
                sleep (1);
#endif
            }
            else
            {
                break;
            }
        }

        if (!wrapper->profile->hasInit ())
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }

    std::atomic<bool> deviceMatch (false);
    int eegChannel = int(wrapper->board_descr["default"]["eeg_channels"].size ());
    wrapper->profile->fetchDeviceInfo (timeout * 1000,
        [&deviceMatch, eegChannel, &wrapper] (DeviceInfo info, string err)
        {
            wrapper->deviceInfo = info;
            if (info.EEGChannelCount == eegChannel)
            {
                deviceMatch = true;
            }
        });
    for (int index = 0; index < timeout; ++index)
    {
        if (!deviceMatch)
        {
#ifdef _WIN32
            Sleep (1000);
#else
            sleep (1);
#endif
        }
        else
        {
            break;
        }
    }
    if (!deviceMatch)
    {
        wrapper->profile->disconnect ();
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }
    SynchroniControllerDelegate::sensorControllerDelegate->logger->log (
        spdlog::level::info, "Session is prepared");
    return 0;
}

int synchroni_start_stream (void *param)
{
    std::tuple<std::string> *info = (std::tuple<std::string> *)(param);
    std::string mac_address = std::get<0> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address] == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    auto wrapper = SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address];
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    if (wrapper->profile == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (!wrapper->profile->hasInit ())
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (wrapper->profile->hasStartDataNotification ())
    {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int timeout = wrapper->params.timeout;
    if (timeout < 1)
    {
        timeout = 5;
    }
    std::atomic<bool> hasSucceed {false};
    std::atomic<bool> finished {false};

    wrapper->profile->startDataNotification (timeout * 1000,
        [&finished, &hasSucceed, &wrapper] (bool succeed, string err)
        {
            hasSucceed = succeed;
            if (err != "")
            {
                SynchroniControllerDelegate::sensorControllerDelegate->logger->log (
                    spdlog::level::err, "Start stream error: " + err);
            }
            wrapper->startStreamTimeStamp = get_timestamp ();
            finished = true;
        });

    for (int index = 0; index < timeout; ++index)
    {
        if (!finished)
        {
#ifdef _WIN32
            Sleep (1000);
#else
            sleep (1);
#endif
        }
        else
        {
            break;
        }
    }
    if (!hasSucceed)
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;

    return 0;
}

int synchroni_stop_stream (void *param)
{
    std::tuple<std::string> *info = (std::tuple<std::string> *)(param);
    std::string mac_address = std::get<0> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address] == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    auto wrapper = SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address];
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    if (wrapper->profile == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (!wrapper->profile->hasInit ())
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int timeout = wrapper->params.timeout;
    if (timeout < 1)
    {
        timeout = 5;
    }
    std::atomic<bool> hasSucceed {false};
    std::atomic<bool> finished {false};

    wrapper->profile->stopDataNotification (timeout * 1000,
        [&finished, &hasSucceed] (bool succeed, string err)
        {
            hasSucceed = succeed;
            if (err != "")
            {
                SynchroniControllerDelegate::sensorControllerDelegate->logger->log (
                    spdlog::level::err, "Stop stream error: " + err);
            }
            finished = true;
        });

    for (int index = 0; index < timeout; ++index)
    {
        if (!finished)
        {
#ifdef _WIN32
            Sleep (1000);
#else
            sleep (1);
#endif
        }
        else
        {
            break;
        }
    }
    if (!hasSucceed)
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;

    return 0;
}

int synchroni_close_device (void *param)
{
    std::tuple<std::string> *info = (std::tuple<std::string> *)(param);
    std::string mac_address = std::get<0> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address] == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    auto wrapper = SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address];
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    if (wrapper->profile == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int timeout = wrapper->params.timeout;
    if (timeout < 1)
    {
        timeout = 5;
    }
    wrapper->profile->disconnect ();
    for (int index = 0; index < timeout; ++index)
    {
        if (wrapper->profile->getDeviceState () != BLEDevice::State::Disconnected)
        {
#ifdef _WIN32
            Sleep (1000);
#else
            sleep (1);
#endif
        }
        else
        {
            break;
        }
    }
    if (wrapper->profile->getDeviceState () != BLEDevice::State::Disconnected)
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;

    return 0;
}
int synchroni_get_data_default (void *param)
{
    std::tuple<std::string, double *, int> *info =
        (std::tuple<std::string, double *, int> *)(param);

    std::string mac_address = std::get<0> (*info);
    double *data = std::get<1> (*info);
    int numOfRows = std::get<2> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address] == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    auto wrapper = SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address];
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    if (wrapper->profile == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (!wrapper->profile->hasStartDataNotification ())
    {
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
    wrapper->dataMutex.lock ();
    if (wrapper->deviceInfo.EEGChannelCount > 0 && wrapper->lastEEG.empty ())
    {
        wrapper->dataMutex.unlock ();
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if (wrapper->deviceInfo.ECGChannelCount > 0 && wrapper->lastECG.empty ())
    {
        wrapper->dataMutex.unlock ();
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    SensorData lastEEG;
    SensorData lastECG;
    if (wrapper->deviceInfo.EEGChannelCount > 0)
    {
        lastEEG = wrapper->lastEEG.at (0);
        wrapper->lastEEG.pop_front ();
    }
    if (wrapper->deviceInfo.ECGChannelCount > 0)
    {
        lastECG = wrapper->lastECG.at (0);
        wrapper->lastECG.pop_front ();
    }
    wrapper->dataMutex.unlock ();

    auto &board_descr = wrapper->board_descr;
    try
    {
        double *board_data = (double *)data;

        if (wrapper->deviceInfo.EEGChannelCount > 0)
        {
            board_data[board_descr["default"]["package_num_channel"].get<int> ()] =
                lastEEG.channelSamples[0][0].sampleIndex;

            double timeStamp =
                wrapper->startStreamTimeStamp + lastEEG.channelSamples[0][0].timeStampInMs / 1000.0;
            board_data[board_descr["default"]["timestamp_channel"].get<int> ()] = timeStamp;

            int eegStart = board_descr["default"]["eeg_channels"][0].get<int> ();
            for (int i = 0; i < lastEEG.channelCount; i++)
            {
                board_data[i + eegStart] = lastEEG.channelSamples[i][0].data;
            }
        }

        if (wrapper->deviceInfo.ECGChannelCount > 0)
        {
            int ecgStart = board_descr["default"]["ecg_channels"][0].get<int> ();
            for (int i = 0; i < lastECG.channelCount; i++)
            {
                board_data[i + ecgStart] = lastECG.channelSamples[i][0].data;
            }
        }
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
int synchroni_release (void *param)
{
    std::tuple<std::string> *info = (std::tuple<std::string> *)(param);
    std::string mac_address = std::get<0> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    SynchroniControllerDelegate::sensorControllerDelegate->boardMap.erase (mac_address);
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    return 0;
}
int synchroni_config_device (void *param)
{
    std::tuple<std::string, std::string, char *, int> *info =
        (std::tuple<std::string, std::string, char *, int> *)(param);

    std::string mac_address = std::get<0> (*info);
    std::string key_char = std::get<1> (*info);
    char *result_char = std::get<2> (*info);
    int buffer_size = std::get<3> (*info);

    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.lock ();
    if (SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address] == NULL)
    {
        SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    auto wrapper = SynchroniControllerDelegate::sensorControllerDelegate->boardMap[mac_address];
    SynchroniControllerDelegate::sensorControllerDelegate->boardMapMutex.unlock ();
    if (wrapper->profile == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int timeout = wrapper->params.timeout;
    if (timeout < 1)
    {
        timeout = 5;
    }
    atomic<bool> finished (false);
    wrapper->profile->setParam (timeout * 1000, key_char, "",
        [&finished, result_char, buffer_size] (string result, string err)
        {
            strncpy (result_char, result.c_str (), buffer_size);
            finished = true;
        });
    for (int index = 0; index < timeout; ++index)
    {
        if (!finished)
        {
#ifdef _WIN32
            Sleep (1000);
#else
            sleep (1);
#endif
        }
        else
        {
            break;
        }
    }
    return 0;
}

#ifdef _WIN32
BOOL WINAPI DllMain (HINSTANCE hinstDLL, // handle to DLL module
    DWORD fdwReason,                     // reason for calling function
    LPVOID lpvReserved)                  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
            // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
            // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:

            if (lpvReserved != nullptr)
            {
                break; // do not do cleanup if process termination scenario
            }
            sensor::SensorController::destory ();
            // Perform any necessary cleanup.
            break;
    }
    return TRUE; // Successful DLL_PROCESS_ATTACH.
}
#endif
