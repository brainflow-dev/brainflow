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
 * \file HubManager.h
 * \brief The Hub factory
 *
 * \version 0.1
 * \date 2017.4.3
 */
#pragma once

#include "gforce.h"
#include "gfTypes.h"
#include "Hub.h"

namespace gf
{

	/// \class HubManager
	/// \brief
	///             The factory of the Hub
	///
	///             HubManater mantains a singleton Hub instance
	///
	class GFORCESDK_API HubManager
	{
	public:
		/// \brief Get the Hub instance
		///
		/// \param sIdentifier a tstring to tell who wants to use the Hub instance
		/// \return The Hub instance
		/// \remark
		///             The life cycle of the Hub instance starts in the first call to this method and until the app exits
		static gfsPtr<Hub> getHubInstance(const tstring& sIdentifier);
	};

} // namespace gf
