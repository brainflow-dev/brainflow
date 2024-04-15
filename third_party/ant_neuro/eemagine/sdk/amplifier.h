/*
 * License: eego amplifier Interface SDK, rev. 1.3
 *
 *
 * Copyright 2018, eemagine Medical Imaging Solutions GmbH
 *
 *
 * 1. Redistributions of source code must retain the copyright notice this list of conditions and
 * the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided with
 * the distribution.
 *
 * 3. All advertising materials mentioning features or use of this software must display the
 * following acknowledgement: This product includes software developed by the eemagine Medical
 * Imaging Solutions GmbH.
 *
 * 4. Neither the name of the eemagine Medical Imaging Solutions GmbH nor the names of its
 * contributors or products may be used to endorse or promote products derived from this software
 * without specific prior written permission by eemagine
 *
 *
 * This Software is provided by eemagine Medical Imaging Solutions GmbH ''As Is'' and any express or
 * implied warranties, including, but not limited to, the implied warranties merchantability and
 * fitness for a particular purpose are disclaimed. In no event shall eemagine be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential damages (including but not
 * limited to, procurement of substitute goods or services, loss of use, data, or profits, or
 * business interruption) however caused and on any theory of liability, whether in contract, strict
 * liability, or tort (including negligence or otherwise) arising in any way out of the use of this
 * software, even if advised of the possibility of such damage.
 *
 * The eego amplifier comes with its own user manual, which must be read carefully before making use
 * of this SDK or any derived software.
 *
 *
 * You need to run full testing and qualification based on your own requirements to claim any
 * performance of the combined system.
 *
 *
 * Please be especially advised that any certification holding for the eego amplifier is not valid
 * for a combined system of your application software and the eego amplifier. You must obtain your
 * own certification for a combined system of amplifier and software.
 */

#ifndef __eemagine_sdk_amplifier_h__
#define __eemagine_sdk_amplifier_h__

// system
#include <cstdint>
#include <string>
#include <vector>
// eemagine
#include <eemagine/sdk/channel.h>
#include <eemagine/sdk/exceptions.h>
#include <eemagine/sdk/stream.h>


namespace eemagine
{
    namespace sdk
    {
        /// representation class for an EEG amplifier
        class amplifier
        {
        public:
            /**
             * destructor
             */
            virtual ~amplifier ()
            {
            }
            /**
             * \brief get a list of all channels available
             * @return list of channels
             */
            virtual std::vector<eemagine::sdk::channel> getChannelList () const = 0;
            /**
             * \brief convenience function to map channel masks to channel list
             * @param ref_mask bitset for selecting which reference channels are used
             * @param bip_mask bitset for selecting which bipolar channels are used
             * @return an list of channels
             */
            std::vector<eemagine::sdk::channel> getChannelList (
                unsigned long long ref_mask, unsigned long long bip_mask)
            {
                int ref_counter (0);
                int bip_counter (0);
                std::vector<eemagine::sdk::channel> rv;
                const std::vector<eemagine::sdk::channel> channel_list (getChannelList ());
                for (size_t n = 0; n < channel_list.size (); ++n)
                {
                    switch (channel_list[n].getType ())
                    {
                        case eemagine::sdk::channel::channel_type::reference:
                            if (ref_mask & ((uint64_t)1 << ref_counter))
                            {
                                rv.push_back (channel_list[n]);
                            }
                            ++ref_counter;
                            break;
                        case eemagine::sdk::channel::channel_type::bipolar:
                            if (bip_mask & ((uint64_t)1 << bip_counter))
                            {
                                rv.push_back (channel_list[n]);
                            }
                            ++bip_counter;
                            break;
                        default:
                            break;
                    }
                }
                return rv;
            }
            /**
             * \brief Returns the serial number of this amplifier
             * @return serial number
             */
            virtual std::string getSerialNumber () const = 0;
            /**
             * \brief Returns the firmware version of this amplifier
             * @return firmware version
             */
            virtual int getFirmwareVersion () const = 0;
            /**
             * \brief Returns the type of this amplifier
             * @return type
             */
            virtual std::string getType () const = 0;
            /**
             * \brief Get the power state
             * @return a structure holding power state, charging state and battery level
             */
            struct power_state
            {
                int is_powered;
                int is_charging;
                int charging_level;
            };
            virtual power_state getPowerState () const = 0;
            /**
             * \brief get list of available sampling rates
             * @return list of sampling rates
             */
            virtual std::vector<int> getSamplingRatesAvailable () const = 0;
            /**
             * \brief get list of available ranges for reference channels
             * @return list of ranges
             */
            virtual std::vector<double> getReferenceRangesAvailable () const = 0;
            /**
             * \brief get list of available ranges for bipolar channels
             * @return list of ranges
             */
            virtual std::vector<double> getBipolarRangesAvailable () const = 0;
            /**
             * \brief Creates an EEG stream
             * @param sampling_rate the sampling rate for this stream, valid values are: 500, 512,
             1000, 1024, 2000, 2048, 4000, 4096, 8000, 8192, 16000, 16384
             * @param reference_range the range, in volt, for the referential channels. Valid values
             are: 1, 0.75, 0.15
             * @param bipolar_range the range, in volt, for the bipolar channels. Valid values are:
             4, 1.5, 0.7, 0.35
             * @param channel_list list of channels to be used
             * @return an object of type stream. The end-user is responsible for deleting the stream
             when done. The data return by the getData call on this streams contains sample values
             measured in Volt. Note that there may only be a maxium one stream alive at all times
             */
            virtual eemagine::sdk::stream *OpenEegStream (int sampling_rate, double reference_range,
                double bipolar_range, const std::vector<eemagine::sdk::channel> &channel_list) = 0;
            /**
             * \brief Overload for creating an EEG stream with default parameters
             * @param sampling_rate the sampling rate for this stream, valid values are: 500, 512,
             1000, 1024, 2000, 2048, 4000, 4096, 8000, 8192, 16000, 16384
             * @return an object of type stream. The end-user is responsible for deleting the stream
             when done. The data return by the getData call on this streams contains sample values
             measured in Volt. Note that there may only be a maxium one stream alive at all times
             */
            eemagine::sdk::stream *OpenEegStream (int sampling_rate, double reference_range,
                double bipolar_range, unsigned long long ref_mask = 0xffffffffffffffff,
                unsigned long long bip_mask = 0xffffffffffffffff)
            {
                return OpenEegStream (sampling_rate, reference_range, bipolar_range,
                    getChannelList (ref_mask, bip_mask));
            }
            /**
             * \brief Overload for creating an EEG stream with default parameters
             * @param sampling_rate the sampling rate for this stream, valid values are: 500, 512,
             1000, 1024, 2000, 2048, 4000, 4096, 8000, 8192, 16000, 16384
             * @return an object of type stream. The end-user is responsible for deleting the stream
             when done. The data return by the getData call on this streams contains sample values
             measured in Volt. Note that there may only be a maxium one stream alive at all times
             */
            eemagine::sdk::stream *OpenEegStream (int sampling_rate)
            {
                return OpenEegStream (sampling_rate, getReferenceRangesAvailable ()[0],
                    getBipolarRangesAvailable ()[0], getChannelList ());
            }
            /**
            * \brief Creates an impedance stream
            * @param channel_list list of channels to be used
            * @return an object of type stream. The end-user is responsible for deleting the stream
            when done. The data return by the getData call on this streams contains sample values
            measured in Ohm. Note that there may only be a maxium one stream alive at all times
            */
            virtual eemagine::sdk::stream *OpenImpedanceStream (
                const std::vector<eemagine::sdk::channel> &channel_list) = 0;
            /**
            * \brief overload for creating an impedance stream with default channel list
            * @return an object of type stream. The end-user is responsible for deleting the stream
            when done. The data return by the getData call on this streams contains sample values
            measured in Ohm. Note that there may only be a maxium one stream alive at all times
            */
            eemagine::sdk::stream *OpenImpedanceStream (
                unsigned long long ref_mask = 0xffffffffffffffff)
            {
                return OpenImpedanceStream (getChannelList (ref_mask, 0));
            }
            /**
             * \brief set battery charging to true or false
             * @param flag true for charging, false for discharging
             * this function may throw an exception if the amplifier does not implement this feature
             */
            virtual void SetBatteryCharging (bool flag) = 0;
            /**
             * \brief set trigger out parameters
             * @param channel output channel
             * @param duty_cycle duty cycle of pulse
             * @param pulse_frequency duration of pulse in Hz
             * @param pulse_count number of pulses
             * @param burst_frequency duration of burst in Hz
             * @param burst_count number of bursts
             * this function may throw an exception if the amplifier does not implement this feature
             */
            virtual void SetTriggerOutParameters (int channel, int duty_cycle,
                float pulse_frequency, int pulse_count, float burst_frequency, int burst_count) = 0;
            /**
             * \brief start trigger out
             * @param channel_list list of channels to start
             * this function may throw an exception if the amplifier does not implement this feature
             */
            virtual void StartTriggerOut (const std::vector<int> &channel_list) = 0;
            /**
             * \brief stop trigger out
             * @param channel_list list of channels to stop
             * this function may throw an exception if the amplifier does not implement this feature
             */
            virtual void StopTriggerOut (const std::vector<int> &channel_list) = 0;
        };
    }
}

#endif
