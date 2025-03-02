#ifndef SENSORCONTROLLER_H
#define SENSORCONTROLLER_H

#include <string>
#include <vector>
#include <memory>
#include "SensorProfile.hpp"

namespace sensor {



class SensorControllerDelegate
{
public:
    virtual void onSensorControllerEnableChanged(bool enabled){};
    virtual void onSensorScanResult(std::vector<BLEDevice> bleDevices){};
    virtual ~SensorControllerDelegate(){};
};

class SensorController
{
public:
    virtual ~SensorController() {};
    static std::shared_ptr<SensorController> getInstance();
    static void destory();
    virtual void setDelegate(std::weak_ptr<SensorControllerDelegate> delegate) = 0;

    virtual bool isEnable() = 0;
    virtual bool isScaning() = 0;
    virtual bool startScan(int periodInMS) = 0;
    virtual bool stopScan() = 0;

    virtual BLEDevice getDevice(std::string deviceMac) = 0;
    virtual std::shared_ptr<SensorProfile> getSensor(std::string deviceMac) = 0;
    virtual std::vector<std::shared_ptr<SensorProfile>> getSensors() = 0;

};

};


#endif // SENSORCONTROLLER_H
