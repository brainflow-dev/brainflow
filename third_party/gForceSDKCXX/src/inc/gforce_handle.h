#pragma once

#include <atlstr.h>
#include <atomic>

#include "gforce.h"
#include "serial_detect.h"


class GForceHandle : public HubListener
{
public:
    GForceHandle (gfsPtr<Hub> &pHub) : mHub (pHub)
    {
        InitializeCriticalSection (&mCriticalSection);
    }
    /// This callback is called when the Hub finishes scanning devices.
    virtual void onScanFinished () override
    {
        if (mDevice == nullptr)
        {
            mExitCode = (int)CustomExitCodes::NO_DEVICE_FOUND;
        }
        else
        {
            DeviceConnectionStatus status = mDevice->getConnectionStatus ();

            if (DeviceConnectionStatus::Connected != status &&
                DeviceConnectionStatus::Connecting != status)
            {
                if (GF_RET_CODE::GF_SUCCESS != mDevice->connect ())
                {
                    mExitCode = (int)CustomExitCodes::CONNECT_ERROR;
                }
                else
                {
                    mExitCode = (int)CustomExitCodes::STATUS_OK;
                }
            }
            else
            {
                mExitCode = (int)CustomExitCodes::STATUS_OK;
            }
        }
    }

    /// This callback is called when the state of the hub changed
    virtual void onStateChanged (HubState state) override
    {
        if (HubState::Disconnected == state)
        {
            mLoop = false;
            mDevice.reset ();
        }
    }

    /// This callback is called when the hub finds a device.
    virtual void onDeviceFound (SPDEVICE device) override
    {
        if (nullptr != device)
        {
            // only search the first connected device if we connected it before
            if (nullptr == mDevice || device == mDevice)
            {
                mDevice = device;
                mHub->stopScan ();
            }
        }
    }

    /// This callback is called a device has been connected successfully
    virtual void onDeviceConnected (SPDEVICE device) override
    {
        if (device)
        {
            static auto setting = device->getDeviceSetting ();

            if (nullptr != setting)
            {
                setting->setDataNotifSwitch (
                    (DeviceSetting::DataNotifFlags) (
                        DeviceSetting::DNF_OFF | DeviceSetting::DNF_EMG_RAW),
                    [](ResponseResult result) {
                        std::string ret =
                            (result == ResponseResult::RREST_SUCCESS) ? ("success") : ("failed");
                    });

                int ret = tryConfigEMG (setting, c_emgConfig[g_EMGchoice].sampleRate,
                    c_emgConfig[g_EMGchoice].resolution);
                if (0 == ret)
                {
                    bGetNewFile = true;
                }
                else
                {
                    if (0 != g_EMGchoice)
                        ret = tryConfigEMG (
                            setting, c_emgConfig[0].sampleRate, c_emgConfig[0].resolution);
                    if (0 == ret)
                        bGetNewFile = true;
                    else
                    {
                        gShouldExit.store (true);
                    }
                }
            }
        }
    }

    /// This callback is called when a device has been disconnected from
    ///                                 connection state or failed to connect to
    virtual void onDeviceDisconnected (SPDEVICE device, GF_UINT8 reason) override
    {
        if (g_file.is_open ())
        {
            g_file.close ();
        }
        mDevice.reset ();
        mHub->startScan (DEFAULT_RSSI_THRESHOLD);
    }

    /// This callback is called when the quaternion data is received
    virtual void onOrientationData (SPDEVICE device, const Quaternion &rotation) override
    {
        // ignore this kind of data
    }

    /// This callback is called when the gesture data is recevied
    virtual void onGestureData (SPDEVICE device, Gesture gest) override
    {
        // ignore this kind of data
    }

    /// This callback is called when the button on gForce is pressed by user
    virtual void onDeviceStatusChanged (SPDEVICE device, DeviceStatus status) override
    {
        // ignore it
    }

    /// This callback is called when extended data is received
    virtual void onExtendedDeviceData (
        SPDEVICE device, DeviceDataType dataType, gfsPtr<const std::vector<GF_UINT8>> data) override
    {
        if (data->size () == 0)
        {
            return;
        }

        switch (dataType)
        {
            case DeviceDataType::DDT_EMGRAW:
            {
                time_t rawtime;
                time (&rawtime);
                // EnterCriticalSection(&mCriticalSection);
                // save raw data with format
                if (g_file.is_open () && bRecording)
                {
                    // format data:0x8192 + CheckSum + PID + RawData + Time
                    g_file.put ((GF_UINT8)0x92); // magic number:0x8192
                    g_file.put ((GF_UINT8)0x81); // magic number:0x8192
                    g_file.put (CheckSum (&(*data)[0], (GF_UINT8)data->size ()));
                    static GF_UINT8 pid = 0;
                    g_file.put (pid);
                    pid++;
                    g_file.write ((const char *)&(*data)[0], data->size ());
                    ULARGE_INTEGER ltime;
                    ltime = GetFileTime ();
                    g_file.write ((const char *)&ltime, sizeof (ltime));
                    printSingleFileRecordedBytes ((int)g_file.tellp ());
                }
                // LeaveCriticalSection(&mCriticalSection);
                gMessageQueue.push (data);
            }
            break;
            default:
                break;
        }
    }

    // Indicates if we want to exit app
    bool getLoop ()
    {
        return mLoop;
    }

    void setLoop ()
    {
        mLoop = true;
    }


private:
    CRITICAL_SECTION mCriticalSection;
    // Indicates if we will keep message polling
    bool mLoop = true;
    // keep a instance of hub.
    gfsPtr<Hub> mHub;
    // keep a device to operate
    gfsPtr<Device> mDevice;
    // exit code
    int mExitCode;

    int tryConfigEMG (gfsPtr<DeviceSetting> &setting, GF_UINT16 samRate, GF_UINT8 resol)
    {

        ResponseResult ret = ResponseResult::RREST_FAILED;
        for (int trytime = 0; trytime < 3; ++trytime)
        {
            std::promise<ResponseResult> emgPromise;
            auto emgFuture = emgPromise.get_future ();
            setting->setEMGRawDataConfig (samRate,            // sample rate
                (DeviceSetting::EMGRowDataChannels) (0x00FF), // channel 0~7
                128,                                          // data length
                resol,                                        // resolution
                [&emgPromise](ResponseResult result) {
                    string retLog =
                        (result == ResponseResult::RREST_SUCCESS) ? ("sucess") : ("failed");
                    emgPromise.set_value (result);
                });
            if (ResponseResult::RREST_SUCCESS == emgFuture.get ())
            {
                ret = ResponseResult::RREST_SUCCESS;
                break;
            }
        }
        if (ResponseResult::RREST_SUCCESS == ret)
            return 0;
        return -1;
    }
};
