#ifndef CPARAMS_H
#define CPARAMS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lib_export.h"
#include <stdbool.h>
#include <stddef.h>

    typedef struct _Device Device;

    typedef enum _FirmwareMode
    {
        FirmwareModeApplication,
        FirmwareModeBootloader
    } FirmwareMode;

    typedef enum _DeviceState
    {
        DeviceStateDisconnected,
        DeviceStateConnected
    } DeviceState;

    typedef enum _SamplingFrequency
    {
        SamplingFrequencyHz125,
        SamplingFrequencyHz250,
        SamplingFrequencyHz500,
        SamplingFrequencyHz1000,
        SamplingFrequencyHz2000,
        SamplingFrequencyHz4000,
        SamplingFrequencyHz8000
    } SamplingFrequency;

    typedef enum _Gain
    {
        Gain1,
        Gain2,
        Gain3,
        Gain4,
        Gain6,
        Gain8,
        Gain12
    } Gain;

    typedef enum _ExternalSwitchInput
    {
        ExternalSwitchInputMioElectrodesRespUSB,
        ExternalSwitchInputMioElectrodes,
        ExternalSwitchInputMioUSB,
        ExternalSwitchInputRespUSB
    } ExternalSwitchInput;

    typedef enum _ADCInput
    {
        ADCInputElectrodes,
        ADCInputShort,
        ADCInputTest,
        ADCInputResistance
    } ADCInput;

    typedef enum _AccelerometerSensitivity
    {
        AccelerometerSens2g,
        AccelerometerSens4g,
        AccelerometerSens8g,
        AccelerometerSens16g
    } AccelerometerSensitivity;

    typedef enum _GyroscopeSensitivity
    {
        GyroscopeSens250Grad,
        GyroscopeSens500Grad,
        GyroscopeSens1000Grad,
        GyroscopeSens2000Grad
    } GyroscopeSensitivity;

    typedef enum _StimulationDeviceState
    {
        StateNoParams,
        StateDisabled,
        StateEnabled
    } StimulationDeviceState;

    typedef struct _StimulatorAndMAState
    {
        StimulationDeviceState StimulatorState;
        StimulationDeviceState MAState;
    } StimulatorAndMaState;

    typedef enum _MotionAssistantLimb
    {

        MotionAssistantLimbRightLeg,
        MotionAssistantLimbLeftLeg,
        MotionAssistantLimbRightArm,
        MotionAssistantLimbLeftArm
    } MotionAssistantLimb;

    typedef struct _MotionAssistantParams
    {
        int gyroStart;
        int gyroStop;
        MotionAssistantLimb limb;
        int minPause;
    } MotionAssistantParams;

    typedef struct _StimulationParams
    {
        int current;
        int pulse_width;
        int frequency;
        int stimulus_duration;
    } StimulationParams;

    typedef struct _FirmwareVersion
    {
        unsigned int version;
        unsigned int build;
    } FirmwareVersion;

#ifdef __cplusplus
}
#endif

#endif // CPARAMS_H
