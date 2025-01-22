#ifndef SENSORPROFILE_H
#define SENSORPROFILE_H

#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include "SensorData.hpp"

namespace sensor {

struct  BLEDevice{
    enum  State{
        Disconnected,
        Connecting,
        Connected,
        Ready,
        Disconnecting,
        Invalid
    };

    std::string name;
    std::string mac;
    int rssi;
};

struct DeviceInfo {
    int MTUSize;
    std::string deviceName;
    std::string modelName;
    std::string hardwareVersion;
    std::string firmwareVersion;
    bool isMTUFine;
    int EEGChannelCount;
    int ECGChannelCount;
    int BRTHChannelCount;
    int AccChannelCount;
    int GyroChannelCount;
};

class SensorProfile;

class SensorProfileDelegate {
public:
    virtual void onErrorCallback(std::shared_ptr<SensorProfile> profile, std::string errorMsg) {};
    virtual void onStateChange(std::shared_ptr<SensorProfile> profile, BLEDevice::State newState) {};
    virtual void onSensorNotifyData(std::shared_ptr<SensorProfile> profile, const SensorData& rawData) {};
    virtual ~SensorProfileDelegate() {};
};

class SensorProfile{
public:
    virtual BLEDevice getDevice() = 0;
    virtual BLEDevice::State getDeviceState() = 0;

    virtual void setDelegate(std::weak_ptr<SensorProfileDelegate> delegate) = 0;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    virtual bool hasStartDataNotification() = 0;
    virtual void startDataNotification(int timeoutInMS, std::function<void(bool result, std::string errorMsg)> cb) = 0;
    virtual void stopDataNotification(int timeoutInMS, std::function<void(bool result, std::string errorMsg)> cb) = 0;

    virtual bool hasInit() = 0;
    virtual void init(int inPackageSampleCount, int timeoutInMS, std::function<void(bool result, std::string errorMsg)> cb) = 0;
    virtual void getBatteryLevel(int timeoutInMS, std::function<void(int result, std::string errorMsg)> cb) = 0;
    virtual void fetchDeviceInfo(int timeoutInMS, std::function<void(DeviceInfo result, std::string errorMsg)> cb) = 0;
    virtual void setParam(int timeoutInMS, std::string key, std::string value, std::function<void(std::string result, std::string errorMsg)> cb) = 0;
    SensorProfile() {};
    virtual ~SensorProfile() {};
};
}

#endif // SENSORPROFILE_H
