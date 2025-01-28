#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <string>
#include <vector>

namespace sensor{

struct SensorData {

    enum Type{
        NTF_ACC_DATA = 1,
        NTF_GYO_DATA = 2,
        NTF_EEG = 16,
        NTF_ECG = 17,
        NTF_IMPEDANCE = 18,
        NTF_IMU = 19,
        NTF_ADS = 20,
        NTF_BRTH = 21,
    };

    std::string deviceMac;
    Type dataType;
    int lastPackageCounter;
    int lastPackageIndex;
    int resolutionBits;
    int sampleRate;
    int channelCount;
    unsigned long long channelMask;
    int minPackageSampleCount;
    int packageSampleCount;
    double K;

    struct Sample {
        int timeStampInMs;
        int channelIndex;
        int sampleIndex;
        int rawData;
        float data;
        float impedance;
        float saturation;
        bool isLost;
    };

    std::vector<std::vector<Sample>> channelSamples;

    void clear() {
        lastPackageCounter = 0;
        lastPackageIndex = 0;
        channelSamples.clear();
    }

};

}

#endif // SENSORDATA_H
