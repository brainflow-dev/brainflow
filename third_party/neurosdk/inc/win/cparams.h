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

typedef enum _FirmwareMode {
	FirmwareModeApplication,
	FirmwareModeBootloader
} FirmwareMode;

typedef enum _DeviceState {
	DeviceStateDisconnected,
	DeviceStateConnected
} DeviceState;

typedef enum _SamplingFrequency {
	SamplingFrequencyHz125,
	SamplingFrequencyHz250,
	SamplingFrequencyHz500,
	SamplingFrequencyHz1000,
	SamplingFrequencyHz2000,
	SamplingFrequencyHz4000,
	SamplingFrequencyHz8000
} SamplingFrequency;

typedef enum _Gain {
	Gain1,
	Gain2,
	Gain3,
	Gain4,
	Gain6,
	Gain8,
	Gain12
} Gain;

typedef enum _ExternalSwitchInput {
	ExternalSwitchInputMioElectrodesRespUSB,
	ExternalSwitchInputMioElectrodes,
	ExternalSwitchInputMioUSB,
	ExternalSwitchInputRespUSB
} ExternalSwitchInput;

typedef enum _ADCInput {
	ADCInputElectrodes,
	ADCInputShort,
	ADCInputTest,
	ADCInputResistance
} ADCInput;

typedef enum _AccelerometerSensitivity {
	AccelerometerSens2g,
	AccelerometerSens4g,
	AccelerometerSens8g,
	AccelerometerSens16g
} AccelerometerSensitivity;

typedef enum _GyroscopeSensitivity{
	GyroscopeSens250Grad,
	GyroscopeSens500Grad,
	GyroscopeSens1000Grad,
	GyroscopeSens2000Grad
} GyroscopeSensitivity;

typedef enum _StimulationDeviceState {
	StateNoParams,
	StateDisabled,
	StateEnabled
} StimulationDeviceState;

typedef struct _StimulatorAndMAState {
	StimulationDeviceState StimulatorState;
	StimulationDeviceState MAState;
} StimulatorAndMaState;

typedef enum _MotionAssistantLimb {

	MotionAssistantLimbRightLeg,
	MotionAssistantLimbLeftLeg,
	MotionAssistantLimbRightArm,
	MotionAssistantLimbLeftArm
} MotionAssistantLimb;

typedef struct _MotionAssistantParams {
	int gyroStart;
	int gyroStop;
	MotionAssistantLimb limb;
	int minPause;
} MotionAssistantParams;

typedef struct _StimulationParams {
	int current;
	int pulse_width;
	int frequency;
	int stimulus_duration;
} StimulationParams;

typedef struct _FirmwareVersion {
	unsigned int version;
	unsigned int build;
} FirmwareVersion;

SDK_SHARED int device_read_Name(Device *, char *out_name, size_t length);
SDK_SHARED int device_read_State(Device *, DeviceState *out_state);
SDK_SHARED int device_read_Address(Device *, char *out_address, size_t length);
SDK_SHARED int device_read_SerialNumber(Device *, char *out_serial, size_t length);
SDK_SHARED int device_read_HardwareFilterState(Device *, bool *out_is_enabled);
SDK_SHARED int device_read_FirmwareMode(Device *, FirmwareMode *out_mode);
SDK_SHARED int device_read_SamplingFrequency(Device *, SamplingFrequency *out_freq);
SDK_SHARED int device_read_Gain(Device *, Gain *out_gain);
SDK_SHARED int device_read_Offset(Device *, unsigned char *out_offset);
SDK_SHARED int device_read_ExternalSwitchState(Device *, ExternalSwitchInput *out_ext_switch);
SDK_SHARED int device_read_ADCInputState(Device *, ADCInput *out_adc_input);
SDK_SHARED int device_read_AccelerometerSens(Device *, AccelerometerSensitivity *out_accel_sens);
SDK_SHARED int device_read_GyroscopeSens(Device *, GyroscopeSensitivity *out_guro_sens);
SDK_SHARED int device_read_StimulatorAndMAState(Device *, StimulatorAndMaState *out_state);
SDK_SHARED int device_read_StimulatorParamPack(Device *, StimulationParams *out_stimul_params);
SDK_SHARED int device_read_MotionAssistantParamPack(Device *, MotionAssistantParams *out_ma_params);
SDK_SHARED int device_read_FirmwareVersion(Device *, FirmwareVersion *out_firmware_version);
SDK_SHARED int device_read_MEMSCalibrationStatus(Device *, bool * out_calibration_status);

SDK_SHARED int device_set_HardwareFilterState(Device *, bool is_enabled);
SDK_SHARED int device_set_FirmwareMode(Device *, FirmwareMode mode);
SDK_SHARED int device_set_SamplingFrequency(Device *, SamplingFrequency freq);
SDK_SHARED int device_set_Gain(Device *, Gain gain);
SDK_SHARED int device_set_Offset(Device *, unsigned char offset);
SDK_SHARED int device_set_ExternalSwitchState(Device *, ExternalSwitchInput ext_switch);
SDK_SHARED int device_set_ADCInputState(Device *, ADCInput adc_input);
SDK_SHARED int device_set_AccelerometerSens(Device *, AccelerometerSensitivity accel_sens);
SDK_SHARED int device_set_GyroscopeSens(Device *, GyroscopeSensitivity gyro_sens);
SDK_SHARED int device_set_StimulatorParamPack(Device *, StimulationParams stimul_params);
SDK_SHARED int device_set_MotionAssistantParamPack(Device *, MotionAssistantParams ma_params);

#ifdef __cplusplus
}
#endif
	
#endif //CPARAMS_H
