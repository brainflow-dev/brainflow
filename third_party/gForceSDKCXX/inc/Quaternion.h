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
 * \file Quaternion.h
 * \brief Quaternion data type
 *
 * \version 0.1
 * \date 2017.4.3
 */
#pragma once

#include <cmath>
#include <sstream>

namespace gf
{

	/// \class Euler
	/// \brief
	///             Data type of Eulerian angle
	///
	class Euler
	{
	public:
		/// \brief Default constructor
		///
		Euler() {}

		/// \brief Constructor with 3 elements
		///
		Euler(float pitch, float roll, float yaw)
			: mPitch(pitch)
			, mRoll(roll)
			, mYaw(yaw)
		{
		}

		/// \brief Get pitch
		///
		/// \return The pitch value
		float pitch() const { return mPitch; }

		/// \brief Get roll
		///
		/// \return The roll value
		float roll() const { return mRoll; }

		/// \brief Get yaw
		///
		/// \return The yaw value
		float yaw() const { return mYaw; }

		/// \brief Get a human-readable value
		///
		/// \return Value in text for human read convenience
		std::string toString() const
		{
			std::ostringstream stm;
			stm << "pitch: " << mPitch << ", roll: " << mRoll << ", yaw: " << mYaw;
			return stm.str();
		}

	private:
		float mPitch = 0;
		float mRoll = 0;
		float mYaw = 0;
	};

	/// \class Quaternion
	/// \brief
	///             Data type of quaternion
	///
	class Quaternion
	{
	public:
		/// \brief Default constructor
		///
		Quaternion() {}

		/// \brief Constructor with 4 elements
		///
		Quaternion(float w, float x, float y, float z)
			: mW(w)
			, mX(x)
			, mY(y)
			, mZ(z)
		{
		}

		/// \brief Get W
		///
		/// \return The W value
		float w() const { return mW; }

		/// \brief Get X
		///
		/// \return The X value
		float x() const { return mX; }

		/// \brief Get Y
		///
		/// \return The Y value
		float y() const { return mY; }

		/// \brief Get Z
		///
		/// \return The Z value
		float z() const { return mZ; }

		/// \brief Get a human-readable value
		///
		/// \return Value in text for human read convenience
		std::string toString() const
		{
			std::ostringstream stm;
			stm << "w: " << mW << ", x: " << mX << ", y: " << mY << ", z: " << mZ;
			return stm.str();
		}

	public:
		/// \brief Convert quaternion to Eulerian angle
		///
		/// \return The converted Eulerian angle
		Euler toEuler() const
		{
			static const float PI = 3.14159265f;
			float pitch = 0, roll = 0, yaw = 0;
			double test = mY*mZ + mX*mW;
			if (std::abs(test) > 0.4999f){
				int symbol = (test > 0.4999f) ? 1 : -1;
				yaw = symbol * 2 * std::atan2f(mY, mW) * 180 / PI;
				pitch = symbol * 90.f;
				roll = 0.f;
				return Euler(pitch, roll, yaw);
			}
			yaw = std::atan2f((2 * mZ*mW - 2 * mX*mY), (1 - 2 * mX*mX - 2 * mZ*mZ)) * 180 / PI;
			pitch = (float)std::asin(2 * test) * 180 / PI;
			roll = std::atan2f((2 * mY*mW - 2 * mX*mZ), (1 - 2 * mX*mX - 2 * mY*mY)) * 180 / PI;
			return Euler(pitch, roll, yaw);
		}

	private:
		float mW = 1;
		float mX = 0;
		float mY = 0;
		float mZ = 0;
	};

} // namespace gf
