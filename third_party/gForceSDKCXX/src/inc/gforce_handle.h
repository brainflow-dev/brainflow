#include <atomic>
#include <deque>
#include <functional>

#include "gforce.h"

#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

#include "brainflow_array.h"
#include "brainflow_constants.h"
#include "spinlock.h"
#include "timestamp.h"

//#define ENABLE_LOGGER

using namespace gf;
using namespace std;

extern volatile int iExitCode;
extern SpinLock spinLock;
extern volatile bool bShouldStopStream;
extern std::deque<BrainFlowArray<double, 1>> dataQueue;


class GforceHandle : public HubListener
{
public:
    GforceHandle (gfsPtr<Hub> &pHub, int iBoardType) : mHub (pHub)
    {
#ifdef ENABLE_LOGGER
        logger = spdlog::stderr_logger_mt ("GForceHandleLogger");
        logger->set_level (spdlog::level::level_enum (0));
        logger->flush_on (spdlog::level::level_enum (0));
#else
        logger = spdlog::create<spdlog::sinks::null_sink_st> ("GForceHandleLogger");
#endif
        bIsEMGConfigured = false;
        bIsFeatureMapConfigured = false;
        iCounter = 0;
        this->iBoardType = iBoardType;
        if (iBoardType == (int)BoardIds::GFORCE_PRO_BOARD)
        {
            iSamplingRate = 500;
            iTransactionSize = 128;
            iNumPackages = 8;
            iChannelMap = 0x00FF;
        }
        if (iBoardType == (int)BoardIds::GFORCE_DUAL_BOARD)
        {
            iSamplingRate = 500;
            iTransactionSize = 32;
            iNumPackages = iTransactionSize / GforceHandle::iADCResolution;
            iChannelMap = 0x0001 | 0x0002;
        }
    }

    /// This callback is called when the Hub finishes scanning devices.
    virtual void onScanFinished () override
    {
        if (nullptr == mDevice)
        {
            logger->error ("device not found");
            iExitCode = (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        else
        {
            DeviceConnectionStatus status = mDevice->getConnectionStatus ();
            if (DeviceConnectionStatus::Connected != status &&
                DeviceConnectionStatus::Connecting != status)
            {
                if (GF_RET_CODE::GF_SUCCESS == mDevice->connect ())
                {
                    logger->info ("device connected");
                    iExitCode = (int)BrainFlowExitCodes::STATUS_OK;
                }
                else
                {
                    logger->error ("connect error");
                    iExitCode = (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
                }
            }
            else
            {
                logger->error ("device found but in connecting state");
                iExitCode = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
            }
        }
    }

    /// This callback is called when the state of the hub changed
    virtual void onStateChanged (HubState state) override
    {
        logger->info ("onStateChanged");
    }

    /// This callback is called when the hub finds a device.
    virtual void onDeviceFound (SPDEVICE device) override
    {
        if (nullptr != device)
        {
            logger->info ("found valid device");
            // only search the first connected device if we connected it before
            if (nullptr == mDevice || device == mDevice)
            {
                mDevice = device;
                mHub->stopScan ();
                logger->info ("stop scanning");
            }
        }
    }

    /// This callback is called if device has been connected successfully
    virtual void onDeviceConnected (SPDEVICE device) override
    {
        logger->info ("onDeviceConnected");
        if (nullptr != device)
        {
            gfsPtr<DeviceSetting> ds = device->getDeviceSetting ();
            if (nullptr != ds)
            {
                ds->getFeatureMap (std::bind (&GforceHandle::featureCallback, this, ds,
                    std::placeholders::_1, std::placeholders::_2));
                logger->info ("set FeatureMap");
            }
        }
    }

    /// This callback is called when a device has been disconnected from connection state or failed
    /// to connect to
    virtual void onDeviceDisconnected (SPDEVICE device, GF_UINT8 reason) override
    {
        // if connection lost, we will try to reconnect again.
        if (nullptr != device && device == mDevice)
        {
            mDevice->connect ();
            logger->info ("reconneting");
        }
    }

    /// This callback is called when the quaternion data is received
    virtual void onOrientationData (SPDEVICE device, const Quaternion &rotation) override
    {
        // dont use this data
        logger->info ("onOrientationData");
    }

    /// This callback is called when the gesture data is recevied
    virtual void onGestureData (SPDEVICE device, Gesture gest) override
    {
        // dont use this data
        logger->info ("onGestureData");
    }

    /// This callback is called when the button on gForce is pressed by user
    virtual void onDeviceStatusChanged (SPDEVICE device, DeviceStatus status) override
    {
        // do nothing
        logger->info ("onDeviceStatusChanged");
    }

    /// This callback is called when extended data is received
    virtual void onExtendedDeviceData (
        SPDEVICE device, DeviceDataType dataType, gfsPtr<const std::vector<GF_UINT8>> data) override
    {
        double timestamp = get_timestamp ();
        if (bShouldStopStream)
        {
            logger->trace ("data received but acqusition is off.");
            return;
        }
        if (!bIsEMGConfigured)
        {
            logger->trace ("EMG is not configured.");
            return;
        }
        if (!bIsFeatureMapConfigured)
        {
            logger->trace ("featureMap is not configured.");
            return;
        }

        if (data->size () != GforceHandle::iTransactionSize)
        {
            return;
        }

        auto ptr = data->data ();
        if (iBoardType == (int)BoardIds::GFORCE_PRO_BOARD)
        {
            constexpr int size = 11;
            double emgData[size] = {0.0};
            if (dataType == DeviceDataType::DDT_EMGRAW)
            {
                for (int packageNum = 0; packageNum < iNumPackages; packageNum++)
                {
                    emgData[0] = iCounter++;
                    for (int i = 0; i < 8; i++)
                    {
                        emgData[i + 1] = (double)*(reinterpret_cast<const uint16_t *> (ptr));
                        ptr += 2;
                    }
                    emgData[9] = timestamp;
                    BrainFlowArray<double, 1> gforceData (emgData, size);
                    spinLock.lock ();
                    dataQueue.push_back (std::move (gforceData));
                    spinLock.unlock ();
                }
            }
        }
        if (iBoardType == (int)BoardIds::GFORCE_DUAL_BOARD)
        {
            constexpr int size = 5;
            double emgData[size] = {0.0};
            if (dataType == DeviceDataType::DDT_EMGRAW)
            {
                emgData[0] = iCounter++;
                for (int packageNum = 0; packageNum < iNumPackages; packageNum++)
                {
                    emgData[0] = iCounter++;
                    for (int i = 0; i < 2; i++)
                    {
                        emgData[i + 1] = (double)*(reinterpret_cast<const uint16_t *> (ptr));
                        ptr += 2;
                    }
                    emgData[3] = timestamp;
                    BrainFlowArray<double, 1> gforceData (emgData, size);
                    spinLock.lock ();
                    dataQueue.push_back (std::move (gforceData));
                    spinLock.unlock ();
                }
            }
        }
    }

    std::shared_ptr<spdlog::logger> logger;
    bool bIsFeatureMapConfigured;
    bool bIsEMGConfigured;
    int iBoardType;
    int iSamplingRate;
    int iTransactionSize;
    int iNumPackages;
    int iChannelMap;

    static const int iADCResolution = 12;

private:
    gfsPtr<Hub> mHub;
    gfsPtr<Device> mDevice;
    int iCounter;

    void featureCallback (gfsPtr<DeviceSetting> ds, ResponseResult res, GF_UINT32 featureMap)
    {
        featureMap >>= 6; // Convert feature map to notification flags

        DeviceSetting::DataNotifFlags flags =
            (DeviceSetting::DataNotifFlags) (DeviceSetting::DNF_OFF
                //| DeviceSetting::DNF_ACCELERATE
                //| DeviceSetting::DNF_GYROSCOPE
                //| DeviceSetting::DNF_MAGNETOMETER
                //| DeviceSetting::DNF_EULERANGLE
                //| DeviceSetting::DNF_QUATERNION
                //| DeviceSetting::DNF_ROTATIONMATRIX
                //| DeviceSetting::DNF_EMG_GESTURE
                | DeviceSetting::DNF_EMG_RAW
                //| DeviceSetting::DNF_HID_MOUSE
                //| DeviceSetting::DNF_HID_JOYSTICK
                | DeviceSetting::DNF_DEVICE_STATUS);

        flags = (DeviceSetting::DataNotifFlags) (flags & featureMap);

        ds->setEMGRawDataConfig (iSamplingRate, (DeviceSetting::EMGRowDataChannels) (iChannelMap),
            iTransactionSize, GforceHandle::iADCResolution,
            [ds, flags, this] (ResponseResult result)
            {
                std::string res =
                    (result == ResponseResult::RREST_SUCCESS) ? ("sucess") : ("failed");
                bIsEMGConfigured = (result == ResponseResult::RREST_SUCCESS) ? (true) : (false);
                this->logger->info ("setEMGRawDataConfig result: {}", res);
                if (bIsEMGConfigured)
                {
                    ds->setDataNotifSwitch (flags,
                        [this] (ResponseResult result)
                        {
                            std::string res =
                                (result == ResponseResult::RREST_SUCCESS) ? ("sucess") : ("failed");
                            bIsFeatureMapConfigured =
                                (result == ResponseResult::RREST_SUCCESS) ? (true) : (false);
                            this->logger->info ("setDataNotifSwitch result: {}", res);
                        });
                }
            });
    }
};
