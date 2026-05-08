#include <simpleble/Descriptor.h>

#include "DescriptorBase.h"

using namespace SimpleBLE;

bool Descriptor::initialized() const { return internal_ != nullptr; }

DescriptorBase* Descriptor::operator->() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

const DescriptorBase* Descriptor::operator->() const {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

BluetoothUUID Descriptor::uuid() { return (*this)->uuid(); }
