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
 * \file Hub.h
 * \brief The abstract of a gForce Hub
 *
 * \version 0.1
 * \date 2017.4.3
 */
#pragma once

#include "gfTypes.h"
#include <vector>
#include <functional>

namespace gf
{
	class HubListener;

	/// \class Hub
	/// \brief
	///             The abstract of a gForce Hub.
	///
	///             The Hub is mantained by HubManager as a singleton instance.
	///             See HubManager for more detail about the life cycle.
	///
	class Hub
	{
	public:
		/// \brief Initialize the Hub instance
		///
		/// \param comPort Specify the COM port the physical Hub is connecting with.
		///                0 means to enumuate COM ports and find one automatically.
		/// \return The result of initalization.
		/// \remark Possible conditions that initialize may fail.
		///   1. The hub is not plugged in the USB port.
		///   2. Other apps are connected to the hub already.
		virtual GF_RET_CODE init(GF_UINT8 comPort = 0) = 0;

		/// \brief De-initialize the Hub instance
		///
		/// \return The result of de-initalization.
		/// \remark During de-initialization, all gForce devices will be disconnected and dropped.
		virtual GF_RET_CODE deinit() = 0;

		/// \brief Get work mode
		///
		/// \return The current work mode
		/// \remark Default work mode is Polling
		virtual WorkMode getWorkMode() const = 0;

		/// \brief Set work mode
		///
		/// \return The current work mode
		/// \remark Default work mode is Polling.\n
		///         See Hub::run and WorkMode also.
		virtual void setWorkMode(WorkMode newMode) = 0;

		/// \brief Get hub state
		///
		/// \return The current hub state
		virtual HubState getState() = 0;

		/// \brief A description text of the hub
		///
		/// \return A text string
		virtual tstring getDescString() const = 0;

		/// \brief Register HubListener
		///
		/// \param listener The client listener to be registered
		/// \return The result of registration.
		virtual GF_RET_CODE registerListener(const gfwPtr<HubListener>& listener) = 0;

		/// \brief Un-register HubListener
		///
		/// \param listener The client listener to be un-registered
		/// \return The result of un-registration.
		virtual GF_RET_CODE unRegisterListener(const gfwPtr<HubListener>& listener) = 0;

		/// \brief Start gForce scan
		///
		/// \param rssiThreshold The threshold to filter devices.\n
		///                      0 - use preset default value.
		/// \return GF_RET_CODE type to indicate if the command sent to hub or not
		virtual GF_RET_CODE startScan(GF_UINT8 rssiThreshold = 0) = 0;

		/// \brief Stop gForce scan
		///
		/// \return GF_RET_CODE type to indicate if the command succeeded or not
		virtual GF_RET_CODE stopScan() = 0;

		/// \brief Get the number of all devices found
		///
		/// \param bConnectedOnly true if only want to get number of connected devices,\n
		/// false if want to get all devices.
		/// \return The Number of all devices
		virtual GF_SIZE getNumOfDevices(bool bConnectedOnly = true) const = 0;

		/// \brief Enumurate devices
		///
		/// \param funEnum The enumurate function
		/// \param bConnectedOnly true if only want to get connected devices,\n
		/// false if want to get all devices.
		/// \remark The return value of funEnum tells gForce if client wants to continue next enumerate
		/// true to continue enumerate, otherwise stop it.
		virtual void enumDevices(std::function<bool(SPDEVICE)>& funEnum, bool bConnectedOnly = true) = 0;

		/// \brief Find a gForce device
		///
		/// \return The weak pointer of the device found, nullptr if not found.
		virtual SPDEVICE findDevice(GF_UINT8 addrType, tstring address) = 0;

		/// \brief run message polling
		///
		/// \param ms The method will return after ms milliseconds. No matter messages are processed or not.
		/// \param once If true, method will return if one message is processed, or ms milliseconds expired.
		/// \return Possible return values:
		///       1. GF_RET_CODE::GF_ERROR_BAD_STATE: the method already been called in other threads, or not in proper WorkMode.
		///       2. GF_RET_CODE::GF_ERROR_TIMEOUT: ms milliseconds expired.
		///       3. GF_RET_CODE::GF_SUCCESS: return with no error.
		/// \remark See more in WorkMode.
		virtual GF_RET_CODE run(GF_UINT32 ms, bool once = false) = 0;

	protected:
		Hub() {}
		virtual ~Hub() {}
	};

} // namespace gf
