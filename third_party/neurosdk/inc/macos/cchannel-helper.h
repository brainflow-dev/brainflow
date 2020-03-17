#ifndef CCHANNELS_HELPER_H
#define CCHANNELS_HELPER_H

#include "sdk_error.h"

#include <memory>
#include "filter/digital_filter.h"
#include "channel_info.h"
#include "cchannels.h"
#include "common_types.h"
#include "event_listener.h"

struct AnyChannelWrapper
{
	using LengthNotifierType = Neuro::Utilities::EventNotifier<Neuro::data_length_t>;
	using LengthListenerType = LengthNotifierType::EventListener;

	virtual ~AnyChannelWrapper() = default;
	virtual Neuro::ChannelInfo& info() noexcept = 0;
	virtual const Neuro::ChannelInfo& info() const noexcept = 0;
	virtual LengthNotifierType& lengthChanged() noexcept = 0;
	virtual Neuro::data_length_t totalLength() const noexcept = 0;
	virtual Neuro::sampling_frequency_t samplingFrequency() const noexcept = 0;
};

template <typename DataT>
struct DataChannelWrapper : public AnyChannelWrapper {
	using DataType = DataT;
	using DataContainer = std::vector<DataType>;

	virtual ~DataChannelWrapper() = default;
	virtual DataContainer readData(Neuro::data_offset_t offset, Neuro::data_length_t length) const = 0;
};

template <typename Channel>
struct SpecificChannelWrapper : public DataChannelWrapper<typename Channel::DataType> {
	using LengthNotifierType = Neuro::Utilities::EventNotifier<size_t>;
	using LengthListenerType = LengthNotifierType::EventListener;
	using DataType = typename Channel::DataType;
	using DataContainer = typename DataChannelWrapper<DataType>::DataContainer;

	explicit SpecificChannelWrapper(std::shared_ptr<Channel> channel):mChannelPtr(channel){}

	Neuro::ChannelInfo& info() noexcept override {
		return mChannelPtr->info();
	}

	const Neuro::ChannelInfo& info() const noexcept override {
		return mChannelPtr->info();
	}

	LengthNotifierType& lengthChanged() noexcept override {
		return mChannelPtr->lengthChanged();
	}

	Neuro::data_length_t totalLength() const noexcept override {
		return mChannelPtr->totalLength();
	}

	Neuro::sampling_frequency_t samplingFrequency() const noexcept override {
		return mChannelPtr->samplingFrequency();
	}

	DataContainer readData(Neuro::data_offset_t offset, Neuro::data_length_t length) const override {
		return mChannelPtr->readData(offset, length);
	}

	std::shared_ptr<Channel> channelPtr() const noexcept {
		return mChannelPtr;
	}

private:
	std::shared_ptr<Channel> mChannelPtr;
};

template <typename CType, typename ChannelPtr>
CType* getCObjectPtr(const ChannelPtr& channelPtr) {
	using ChannelWrap = SpecificChannelWrapper<typename ChannelPtr::element_type>;
	using ChannelWrapPtr = std::shared_ptr<ChannelWrap>;
	auto channelWrapPtr = new ChannelWrapPtr(new ChannelWrap(channelPtr));
	return reinterpret_cast<CType *>(channelWrapPtr);
}

SDK_SHARED std::unique_ptr<DSP::DigitalFilter<double>> createFilter(Filter filter);

SDK_SHARED std::unique_ptr<DSP::DigitalFilter<double>> getCompoundFilter(Filter *filters, size_t filter_count);

template <typename Channel>
int readChannelData(const Channel &channel, size_t offset, size_t length, typename Channel::DataType *out_buffer, size_t buffer_size, size_t *samples_read) {
	try {
		auto data = channel.readData(offset, length);
		if (data.size() > buffer_size) {
			set_sdk_last_error("Read data length is greater than read buffer size");
			return ERROR_EXCEPTION_WITH_MESSAGE;
		}
		std::copy(data.begin(), data.end(), out_buffer);
		*samples_read = data.size();
		return SDK_NO_ERROR;
	}
	catch (std::exception &e) {
		set_sdk_last_error(e.what());
		return ERROR_EXCEPTION_WITH_MESSAGE;
	}
	catch (...) {
		return ERROR_UNHANDLED_EXCEPTION;
	}
}

template<typename Channel>
SDK_SHARED int readBufferSize(const Channel &channel, size_t* out_buffer_size) {
	try {
		*out_buffer_size = channel.bufferSize();
		return SDK_NO_ERROR;
	}
	catch (std::exception &e) {
		set_sdk_last_error(e.what());
		return ERROR_EXCEPTION_WITH_MESSAGE;
	}
	catch (...) {
		return ERROR_UNHANDLED_EXCEPTION;
	}
}

template <typename Channel>
int readTotalLength(const Channel &channel, size_t* out_length) {
	try {
		*out_length = channel.totalLength();
		return SDK_NO_ERROR;
	}
	catch (std::exception &e) {
		set_sdk_last_error(e.what());
		return ERROR_EXCEPTION_WITH_MESSAGE;
	}
	catch (...) {
		return ERROR_UNHANDLED_EXCEPTION;
	}
}

template <typename Channel>
int readSamplingFrequency(const Channel &channel, float* out_frequency) {
	try {
		*out_frequency = channel.samplingFrequency();
		return SDK_NO_ERROR;
	}
	catch (std::exception &e) {
		set_sdk_last_error(e.what());
		return ERROR_EXCEPTION_WITH_MESSAGE;
	}
	catch (...) {
		return ERROR_UNHANDLED_EXCEPTION;
	}
}

template <typename Channel>
int getChannelInfo(Channel &channel, ChannelInfo *out_info) {
	try {
		auto channelInfo = channel.info();
		ChannelInfo info;
		strcpy(info.name, channelInfo.getName().c_str());
		info.type = static_cast<ChannelType>(channelInfo.getType());
		info.index = channelInfo.getIndex();
		*out_info = info;
		return SDK_NO_ERROR;
	}
	catch (std::exception &e) {
		set_sdk_last_error(e.what());
		return ERROR_EXCEPTION_WITH_MESSAGE;
	}
	catch (...) {
		return ERROR_UNHANDLED_EXCEPTION;
	}
}


#endif