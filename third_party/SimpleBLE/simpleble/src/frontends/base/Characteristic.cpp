#include <simpleble/Characteristic.h>

#include "BuildVec.h"
#include "CharacteristicBase.h"

using namespace SimpleBLE;

BluetoothUUID Characteristic::uuid() { return (*this)->uuid(); }

std::vector<Descriptor> Characteristic::descriptors() { return Factory::vector((*this)->descriptors()); }

std::vector<std::string> Characteristic::capabilities() {
    std::vector<std::string> capabilities;

    if (can_read()) {
        capabilities.push_back("read");
    }

    if (can_write_request()) {
        capabilities.push_back("write_request");
    }

    if (can_write_command()) {
        capabilities.push_back("write_command");
    }

    if (can_notify()) {
        capabilities.push_back("notify");
    }

    if (can_indicate()) {
        capabilities.push_back("indicate");
    }

    return capabilities;
}

bool Characteristic::initialized() const { return internal_ != nullptr; }

CharacteristicBase* Characteristic::operator->() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

const CharacteristicBase* Characteristic::operator->() const {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

bool Characteristic::can_read() { return (*this)->can_read(); }
bool Characteristic::can_write_request() { return (*this)->can_write_request(); }
bool Characteristic::can_write_command() { return (*this)->can_write_command(); }
bool Characteristic::can_notify() { return (*this)->can_notify(); }
bool Characteristic::can_indicate() { return (*this)->can_indicate(); }
