/*
 * Copyright 2017, OYMotion Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */

#include <atomic>
#include <functional>

#include "gforce.h"

#include "gforce_wrapper_types.h"

using namespace gf;
using namespace std;

#ifdef DEBUG
#include <iostream>
#endif


class GForceHandle : public HubListener
{
public:
    GForceHandle (gfsPtr<Hub> &pHub) : mHub (pHub)
    {
    }
    /// This callback is called when the Hub finishes scanning devices.
    virtual void onScanFinished () override
    {
        if (nullptr == mDevice)
        {
            iExitCode = (int)CustomExitCodes::NO_DEVICE_FOUND;
        }
        else
        {
            DeviceConnectionStatus status = mDevice->getConnectionStatus ();
            if (DeviceConnectionStatus::Connected != status &&
                DeviceConnectionStatus::Connecting != status)
            {
                if (GF_RET_CODE::GF_SUCCESS == mDevice->connect ())
                {
                    iExitCode = (int)CustomExitCodes::STATUS_OK;
                }
                else
                {
                    iExitCode = (int)CustomExitCodes::CONNECT_ERROR;
                }
            }
            else
            {
                iExitCode = (int)CustomExitCodes::FOUND_BUT_IN_CONNECTING_STATE;
            }
        }
    }

    /// This callback is called when the state of the hub changed
    virtual void onStateChanged (HubState state) override
    {
        // do nothing
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
        if (nullptr != device)
        {
            gfsPtr<DeviceSetting> ds = device->getDeviceSetting ();
            if (nullptr != ds)
            {
                ds->getFeatureMap (std::bind (&GForceHandle::featureCallback, this, ds,
                    std::placeholders::_1, std::placeholders::_2));
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
        }
    }

    /// This callback is called when the quaternion data is received
    virtual void onOrientationData (SPDEVICE device, const Quaternion &rotation) override
    {
        // dont use this data
    }

    /// This callback is called when the gesture data is recevied
    virtual void onGestureData (SPDEVICE device, Gesture gest) override
    {
        // dont use this data
    }

    /// This callback is called when the button on gForce is pressed by user
    virtual void onDeviceStatusChanged (SPDEVICE device, DeviceStatus status) override
    {
        // do nothing
    }

    /// This callback is called when extended data is received
    virtual void onExtendedDeviceData (
        SPDEVICE device, DeviceDataType dataType, gfsPtr<const std::vector<GF_UINT8>> data) override
    {
#ifdef DEBUG
        cout << __FUNCTION__ << ": datatype = " << (GF_UINT32)dataType
             << ", datalength = " << data->size () << ", first byte: " << hex
             << (GF_UINT32) ((data->size () > 0) ? data->at (0) : 0xFF) << ", last byte: "
             << (GF_UINT32) ((data->size () > 0) ? data->at (data->size () - 1) : 0xFF) << dec
             << endl;
#endif

        if (data->size () == 0)
        {
            return;
        }

        switch (dataType)
        {
            case DeviceDataType::DDT_ACCELERATE:
                break;

            case DeviceDataType::DDT_GYROSCOPE:
                break;

            case DeviceDataType::DDT_MAGNETOMETER:
                break;

            case DeviceDataType::DDT_EULERANGLE:
                break;

            case DeviceDataType::DDT_QUATERNION:
                break;

            case DeviceDataType::DDT_ROTATIONMATRIX:
                break;

            case DeviceDataType::DDT_GESTURE:
                break;

            case DeviceDataType::DDT_EMGRAW:
                break;

            case DeviceDataType::DDT_HIDMOUSE:
                break;

            case DeviceDataType::DDT_HIDJOYSTICK:
                break;

            case DeviceDataType::DDT_DEVICESTATUS:
                break;
        }
    }

private:
    // keep a instance of hub.
    gfsPtr<Hub> mHub;
    // keep a device to operate
    gfsPtr<Device> mDevice;

    void featureCallback (gfsPtr<DeviceSetting> ds, ResponseResult res, GF_UINT32 featureMap)
    {
        featureMap >>= 6; // Convert feature map to notification flags

        DeviceSetting::DataNotifFlags flags = (DeviceSetting::DataNotifFlags) (
            DeviceSetting::DNF_OFF | DeviceSetting::DNF_ACCELERATE | DeviceSetting::DNF_GYROSCOPE |
            DeviceSetting::DNF_MAGNETOMETER
            //| DeviceSetting::DNF_EULERANGLE
            //| DeviceSetting::DNF_QUATERNION
            //| DeviceSetting::DNF_ROTATIONMATRIX
            //| DeviceSetting::DNF_EMG_GESTURE
            | DeviceSetting::DNF_EMG_RAW
            //| DeviceSetting::DNF_HID_MOUSE
            //| DeviceSetting::DNF_HID_JOYSTICK
            | DeviceSetting::DNF_DEVICE_STATUS);

        ds->setDataNotifSwitch (
            (DeviceSetting::DataNotifFlags) (flags & featureMap), [](ResponseResult res) {});

        ds->setEMGRawDataConfig (500,                     // sample rate
            (DeviceSetting::EMGRowDataChannels) (0x00FF), // channel 0~7
            128,                                          // data length
            12,                                           // adc resolution
            [](ResponseResult result) {});
    }
};
