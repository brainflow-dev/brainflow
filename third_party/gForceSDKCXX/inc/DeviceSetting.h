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
/*!
* \file DeviceSetting.h
* \brief The abstract of the device setting handler of a gForce device
*
* \version 0.1
* \date 2017.8.3
*/
#pragma once

#include "gfTypes.h"
#include <cstdio>
#include <functional>

namespace gf
{
	enum class ResponseResult : GF_UINT32 {
		RREST_SUCCESS,
		RREST_NOT_SUPPORT,
		RREST_BAD_PARAM,
		RREST_FAILED,
		RREST_TIMEOUT,
	};

	/// \class DeviceSetting
	/// \brief
	///     The abstract of the device setting handler of a gForce device
	///
	///     According to 4.x definition.
	///
	class DeviceSetting
	{
	public:
		/////////////////////////////////////////////////////
		// description
		virtual GF_RET_CODE getProtocolVer(std::function<void(ResponseResult res, tstring version)> cb) = 0;
		virtual GF_RET_CODE getFeatureMap(std::function<void(ResponseResult res, GF_UINT32 featureMap)> cb) = 0;
		virtual GF_RET_CODE getDeviceName(std::function<void(ResponseResult res, tstring name)> cb) = 0;
		virtual GF_RET_CODE getModelNumber(std::function<void(ResponseResult res, tstring modelNumber)> cb) = 0;
		virtual GF_RET_CODE getSerialNumber(std::function<void(ResponseResult res, tstring serialNumber)> cb) = 0;
		virtual GF_RET_CODE getHWRevision(std::function<void(ResponseResult res, tstring version)> cb) = 0;
		virtual GF_RET_CODE getFWRevision(std::function<void(ResponseResult res, tstring version)> cb) = 0;
		virtual GF_RET_CODE getManufacturerName(std::function<void(ResponseResult res, tstring name)> cb) = 0;
		virtual GF_RET_CODE getBootloaderVer(std::function<void(ResponseResult res, tstring version)> cb) = 0;

	public:
		/////////////////////////////////////////////////////
		// control
		virtual GF_RET_CODE powerOff() = 0;
		virtual GF_RET_CODE swithToOAD() = 0;
		virtual GF_RET_CODE systemReset() = 0;
		
		enum class DeviceService : GF_UINT32 {
			/// HID Joystick Service
			HID_Joystick = 0x1,
			/// HID Mouse Service
			HID_Mouse = 0x2,
			/// HID Vendor Service
			HID_Vendor = 0x3,
		};
		// C.2
		virtual GF_RET_CODE switchService(DeviceService service) = 0;
		virtual GF_RET_CODE oadUpgrade(FILE* file, std::function<void(ResponseResult res, GF_UINT32 percentage)> progress) = 0;

	public:
		/////////////////////////////////////////////////////
		// config
		// C.6
		virtual GF_RET_CODE sendTrainingModelData(GF_UINT32 length, GF_UINT8 data[], std::function<void(ResponseResult res, GF_UINT32 percentage)> progress) = 0;

		enum DataNotifFlags {
			/// Data Notify All Off
			DNF_OFF = 0x00000000,
			/// Accelerate On(C.7)
			DNF_ACCELERATE = 0x00000001,
			/// Gyroscope On(C.8)
			DNF_GYROSCOPE = 0x00000002,
			/// Magnetometer On(C.9)
			DNF_MAGNETOMETER = 0x00000004,
			/// Euler Angle On(C.10)
			DNF_EULERANGLE = 0x00000008,
			/// Quaternion On(C.11)
			DNF_QUATERNION = 0x00000010,
			/// Rotation Matrix On(C.12)
			DNF_ROTATIONMATRIX = 0x00000020,
			/// EMG Gesture On(C.13)
			DNF_EMG_GESTURE = 0x00000040,
			/// EMG Raw Data On(C.14)
			DNF_EMG_RAW = 0x00000080,
			/// HID Mouse On(C.15)
			DNF_HID_MOUSE = 0x00000100,
			/// HID Joystick On(C.16)
			DNF_HID_JOYSTICK = 0x00000200,
			/// Device Status On(C.17)
			DNF_DEVICE_STATUS = 0x00000400,
			/// Data Notify All On
			DNF_ALL = 0xFFFFFFFF,
		};
		virtual GF_RET_CODE setDataNotifSwitch(DataNotifFlags flags, std::function<void(ResponseResult res)> cb) = 0;

		///////////////////////////////////////////////
		virtual GF_RET_CODE getBatteryLevel( std::function<void(ResponseResult res, GF_UINT32 percentage)> cb) = 0;
		// C.1
		virtual GF_RET_CODE getTemperature(std::function<void(ResponseResult res, GF_UINT32 temperature)> cb) = 0;

		enum class SWOLogLevel : GF_UINT32 {
			/// All
			All = 0x00,
			/// Debug
			Debug = 0x01,
			/// Info
			Info = 0x02,
			/// Warn
			Warn = 0x03,
			/// Error
			Error = 0x04,
			/// Fatal
			Fatal = 0x05,
			/// None
			None = 0xFF,
		};
		// C.3
		virtual GF_RET_CODE setLogLevel(SWOLogLevel level, std::function<void(ResponseResult res)> cb) = 0;

		enum SWOModule : GF_UINT32 {
			/// None Module Enable
			SWO_MODULE_NONE = 0x00000000,
			/// Npi Module Enable
			SWO_MODULE_NPI = 0x00000001,
			/// IMU Module Enable
			SWO_MODULE_IMU = 0x00000002,
			/// RTOS Module Enable
			SWO_MODULE_RTOS = 0x00000004,
			/// ADC Module Enable
			SWO_MODULE_ADC = 0x00000008,
			/// Profile Module Enable
			SWO_MODULE_PROFILE = 0x00000010,
			/// App Module Enable
			SWO_MODULE_APP = 0x00000020,
			/// All Modules Enable
			SWO_MODULE_ALL = 0xFFFFFFFF,
		};
		// C.3
		virtual GF_RET_CODE setLogModuleEnabled(SWOModule modules, std::function<void(ResponseResult res)> cb) = 0;

		enum class KernelMsgType : GF_UINT8 {
			/// Print Task List
			TaskList = 0x00,
			/// Print Runtime State
			RuntimeState = 0x01,
		};
		// C.3
		virtual GF_RET_CODE printKernelMsg(KernelMsgType type, std::function<void(ResponseResult res)> cb) = 0;

		enum class VibrateControlType : GF_UINT8 {
			/// Vibrate Off Test
			Off = 0x00,
			/// Vibrate On Test
			On = 0x01,
			/// Device Vibrate Disable
			Disable = 0x02,
			/// Device Vibrate Enable
			Enable = 0x03,
		};
		// C.4
		virtual GF_RET_CODE vibrateControl(VibrateControlType type, std::function<void(ResponseResult res)> cb) = 0;

		enum class LedControlTestType : GF_UINT8 {
			/// LED Off Test
			Off = 0x00,
			/// LED On Test
			On = 0x01,
		};
		// C.5
		virtual GF_RET_CODE ledControlTest(LedControlTestType type, std::function<void(ResponseResult res)> cb) = 0;

		enum class PackageControlType : GF_UINT8 {
			/// Package ID Disable
			Disable = 0x00,
			/// Package ID Enable
			Enable = 0x01,
		};
		virtual GF_RET_CODE packageIdControl(PackageControlType type, std::function<void(ResponseResult res)> cb) = 0;

		// C.7
		virtual GF_RET_CODE getAccelerateCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz,
			GF_UINT8 maxScaleRange_g, GF_UINT8 maxPackageDataLength)> cb) = 0;
		virtual GF_RET_CODE setAccelerateConfig(GF_UINT16 sampleRateHz,
			GF_UINT8 fullScaleRange_g, GF_UINT8 packageDataLength, std::function<void(ResponseResult res)> cb) = 0;
		// C.8
		virtual GF_RET_CODE getGyroscopeCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz,
			GF_UINT16 maxScaleRange_dps, GF_UINT8 maxPackageDataLength)> cb) = 0;
		virtual GF_RET_CODE setGyroscopeConfig(GF_UINT16 sampleRateHz,
			GF_UINT16 fullScaleRange_dps, GF_UINT8 packageDataLength, std::function<void(ResponseResult res)> cb) = 0;
		// C.9
		virtual GF_RET_CODE getMagnetometerCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz,
			GF_UINT16 maxScaleRange_uT, GF_UINT8 maxPackageDataLength)> cb) = 0;
		virtual GF_RET_CODE setMagnetometerConfig(GF_UINT16 sampleRateHz,
			GF_UINT16 fullScaleRange_uT, GF_UINT8 packageDataLength, std::function<void(ResponseResult res)> cb) = 0;
		// C.10
		virtual GF_RET_CODE getEulerangleCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz)> cb) = 0;
		virtual GF_RET_CODE setEulerangleConfig(GF_UINT16 sampleRateHz, std::function<void(ResponseResult res)> cb) = 0;
		// C.11
		virtual GF_RET_CODE getQuaternionCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz)> cb) = 0;
		virtual GF_RET_CODE setQuaternionConfig(GF_UINT16 sampleRateHz, std::function<void(ResponseResult res)> cb) = 0;
		// C.12
		virtual GF_RET_CODE getRotationMatrixCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz)> cb) = 0;
		virtual GF_RET_CODE setRotationMatrixConfig(GF_UINT16 sampleRateHz, std::function<void(ResponseResult res)> cb) = 0;
		// C.13
		virtual GF_RET_CODE getGestureCap(std::function<void(ResponseResult res, GF_SIZE number, const Gesture supportedGestures[])> cb) = 0;
		virtual GF_RET_CODE setGestureConfig(GF_UINT32 number, Gesture interestingGesture[], std::function<void(ResponseResult res)> cb) = 0;

		enum EMGRowDataChannels : GF_UINT16 {
			ERDC_CHANNEL_NONE = 0x0000, /// None Channel supported
			ERDC_CHANNEL0 = 0x0001, /// Channel 0 supported
			ERDC_CHANNEL1 = 0x0002, /// Channel 1 supported
			ERDC_CHANNEL2 = 0x0004, /// Channel 2 supported
			ERDC_CHANNEL3 = 0x0008, /// Channel 3 supported
			ERDC_CHANNEL4 = 0x0010, /// Channel 4 supported
			ERDC_CHANNEL5 = 0x0020, /// Channel 5 supported
			ERDC_CHANNEL6 = 0x0040, /// Channel 6 supported
			ERDC_CHANNEL7 = 0x0080, /// Channel 7 supported
			ERDC_CHANNEL8 = 0x0100, /// Channel 8 supported
			ERDC_CHANNEL9 = 0x0200, /// Channel 9 supported
			ERDC_CHANNEL10 = 0x0400, /// Channel 10 supported
			ERDC_CHANNEL11 = 0x0800, /// Channel 11 supported
			ERDC_CHANNEL12 = 0x1000, /// Channel 12 supported
			ERDC_CHANNEL13 = 0x2000, /// Channel 13 supported
			ERDC_CHANNEL14 = 0x4000, /// Channel 14 supported
			ERDC_CHANNEL15 = 0x8000, /// Channel 15 supported
			ERDC_CHANNEL_ALL = 0xFFFF, /// All Channels supported
		};
		// C.14
		virtual GF_RET_CODE getEMGRawDataCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz,
			EMGRowDataChannels supportedChannels, GF_UINT8 maxPackageDataLength)> cb) = 0;
		virtual GF_RET_CODE setEMGRawDataConfig(GF_UINT16 sampleRateHz,
			EMGRowDataChannels interestingChannels, GF_UINT8 packageDataLength, GF_UINT8 adcResolution,
			std::function<void(ResponseResult res)> cb) = 0;
		// C.15
		virtual GF_RET_CODE getMouseDataCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz)> cb) = 0;
		virtual GF_RET_CODE setMouseDataConfig(GF_UINT16 sampleRateHz, std::function<void(ResponseResult res)> cb) = 0;
		// C.16
		virtual GF_RET_CODE getJoystickDataCap(std::function<void(ResponseResult res, GF_UINT16 maxSampleRateHz)> cb) = 0;
		virtual GF_RET_CODE setJoystickDataConfig(GF_UINT16 sampleRateHz, std::function<void(ResponseResult res)> cb) = 0;

		enum DeviceStatusFlags : GF_UINT16 {
			/// None Status Event supported
			DSF_NONE = 0x0000,
			/// Re-center Status Event supported
			DSF_RECENTER = 0x0001,
			/// USB plug/unplug Event supported
			DSF_USBSTATUS = 0x0002,
			/// Motionless Status Event supported
			DSF_MOTIONLESS = 0x0004,
		};
		// C.17
		virtual GF_RET_CODE getDeviceStatusCap(std::function<void(ResponseResult res, DeviceStatusFlags flags)> cb) = 0;
		virtual GF_RET_CODE setDeviceStatusConfig(DeviceStatusFlags flags, std::function<void(ResponseResult res)> cb) = 0;

	protected:
		/// \brief Virtual deconstructor
		///
		virtual ~DeviceSetting()
		{
		}
	};

}
