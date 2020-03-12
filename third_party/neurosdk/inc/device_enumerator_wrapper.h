#ifndef DEVICE_ENUMERATOR_WRAPPER_H
#define DEVICE_ENUMERATOR_WRAPPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "device_info.h"
#include "device.h"
#include "event_notifier.h"
#include <vector>

struct DeviceEnumeratorWrapper {
	virtual ~DeviceEnumeratorWrapper() = default;
	virtual std::vector<Neuro::DeviceInfo> devices() const = 0;
	virtual Neuro::Utilities::EventNotifier<>& deviceListChanged() = 0;
	virtual std::shared_ptr<Neuro::Device> createDevice(const Neuro::DeviceInfo &) = 0;
};

#ifdef __cplusplus
}
#endif
	
#endif // DEVICE_ENUMERATOR_WRAPPER_H
