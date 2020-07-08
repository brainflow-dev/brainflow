#ifndef CDEVICE_H
#define CDEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "clistener.h"
#include "cscanner.h"
#include "lib_export.h"
#include <stdbool.h>
#include <stddef.h>

    typedef struct _Device Device;

    typedef enum _Command
    {
        CommandStartSignal,
        CommandStopSignal,
        CommandStartResist,
        CommandStopResist,
        CommandStartMEMS,
        CommandStopMEMS,
        CommandStartRespiration,
        CommandStopRespiration,
        CommandStartStimulation,
        CommandStopStimulation,
        CommandEnableMotionAssistant,
        CommandDisableMotionAssistant,
        CommandFindMe,
        CommandStartAngle,
        CommandStopAngle,
        CommandCalibrateMEMS,
        CommandResetQuaternion
    } Command;

    typedef struct _CommandArray
    {
        Command *cmd_array;
        size_t cmd_array_size;
    } CommandArray;

    typedef enum _Parameter
    {
        ParameterName,
        ParameterState,
        ParameterAddress,
        ParameterSerialNumber,
        ParameterHardwareFilterState,
        ParameterFirmwareMode,
        ParameterSamplingFrequency,
        ParameterGain,
        ParameterOffset,
        ParameterExternalSwitchState,
        ParameterADCInputState,
        ParameterAccelerometerSens,
        ParameterGyroscopeSens,
        ParameterStimulatorAndMAState,
        ParameterStimulatorParamPack,
        ParameterMotionAssistantParamPack,
        ParameterFirmwareVersion,
        ParameterMEMSCalibrationStatus
    } Parameter;

    typedef enum _ParamAccess
    {
        Read,
        ReadWrite,
        ReadNotify
    } ParamAccess;

    typedef struct _ParameterInfo
    {
        Parameter parameter;
        ParamAccess access;
    } ParameterInfo;

    typedef struct _ParamInfoArray
    {
        ParameterInfo *info_array;
        size_t info_count;
    } ParamInfoArray;

    typedef enum _ChannelType
    {
        ChannelTypeSignal,
        ChannelTypeBattery,
        ChannelTypeElectrodesState,
        ChannelTypeRespiration,
        ChannelTypeMEMS,
        ChannelTypeOrientation,
        ChannelTypeConnectionStats,
        ChannelTypeResistance,
        ChannelTypePedometer,
        ChannelTypeCustom
    } ChannelType;

    typedef struct _ChannelInfo
    {
        char name[128];
        ChannelType type;
        size_t index;
    } ChannelInfo;

    typedef struct _ChanInfoArray
    {
        ChannelInfo *info_array;
        size_t info_count;
    } ChannelInfoArray;

    typedef struct _DoubleDataArray
    {
        double *data_array;
        size_t samples_count;
    } DoubleDataArray;

    typedef struct _IntDataArray
    {
        int *data_array;
        size_t samples_count;
    } IntDataArray;

#ifdef __cplusplus
}
#endif

#endif // CDEVICE_H
