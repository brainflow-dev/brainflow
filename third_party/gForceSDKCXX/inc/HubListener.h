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
 * \file HubListener.h
 * \brief The definition of gForce callbacks
 *
 * \version 0.1
 * \date 2017.4.3
 */
#pragma once

#include "gfTypes.h"
#include "Quaternion.h"

#include <vector>

namespace gf
{

	/// \class HubListener
	/// \brief
	///     The callback interface for processing messages from the Hub.
	///
	///     The application needs to implement this class and pass its instance
	///     to Hub::registerListener()
	///
	class HubListener
	{
	public:
		/// \brief This callback is called when the Hub finishes scanning devices.
		/// \remark
		///     This callback may be called after a series of onDeviceFound()
		///     are called.
		virtual void onScanFinished() {}

		/// \brief This callback is called when the state of the hub changed
		/// \param state
		///     An enumerated value of HubState which indicates the state of the
        ///     hub.
		virtual void onStateChanged(HubState state) {}

		/// \brief
        ///     This callback is called when the hub finds a device during
        ///     scanning.
		/// \param device The pointer to a Device object that was found.
		virtual void onDeviceFound(SPDEVICE device) {}

		/// \brief
        ///     This callback is called when a previously found but not
        ///     connected device has been dropped by the Hub.
		/// \param device
        ///     The pointer to the Device object that was previously found and
		///     passed to the application.
		virtual void onDeviceDiscard(SPDEVICE device) {}

		/// \brief
        ///     This callback is called when a device has been connected to
        ///     the hub successfully.
		/// \param device
        ///     The pointer to the Device object that the hub has connected to.
		virtual void onDeviceConnected(SPDEVICE device) {}

		/// \brief
        ///     This callback is called when a device has been disconnected from
		///     connection state or failed to connect to
		/// \param device
        ///     The pointer to the Device object that was disconnected.
		/// \param reason The reason of why device disconnected.
		virtual void onDeviceDisconnected(SPDEVICE device, GF_UINT8 reason) {}

		/// \brief
        ///     This callback is called when the orientation of the device has
        ///     changed.
		/// \param device
        ///     The pointer to the Device.
		/// \param orientation
        ///     The Quaternion object that indicates the updated orientation of
        ///     the device.
		virtual void onOrientationData(SPDEVICE device, const Quaternion& orientation) {}

		/// \brief This callback is called when the gesture data is recevied
		/// \param device The Pointer to the Device.
		/// \param gest The Gesture object.
		virtual void onGestureData(SPDEVICE device, Gesture gest) {}

		/// \brief
        ///     This callback is called when the device status has been changed
		/// \param device Pointer to the Device sending data
		/// \param status What status of the device has been changed
        /// \remark
        ///     DeviceStatus::ReCenter: Notifiy the application of that the user has
        ///     re-centered the device, by pressing the button on the gForce
        ///     device, to use his/her current orientation as the origin of
        ///     his/her coordinate system (i.e. [w=1, x=0, y=0, z=0] in
        ///     quaternion). Please also see <a href="https://support.google.com/daydream/answer/7184599?hl=en">Google Daydram - Re-center your view & cursor</a>
		///     DeviceStatus::Motionless: Notify that the device doesn't detect user motion
		///     in long time, so it will turn to standby mode.
		virtual void onDeviceStatusChanged(SPDEVICE device, DeviceStatus status) {}

		/// \brief This callback is called when the extended data is recevied
		/// \param device The Pointer to the Device.
		/// \param dataType The data type carried
		/// \param data The data buffer managed by reference
		virtual void onExtendedDeviceData(SPDEVICE device, DeviceDataType dataType, gfsPtr<const std::vector<GF_UINT8>> data) {}

		virtual ~HubListener() {}
	};
}
