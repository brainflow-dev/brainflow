#include <simpleble/Characteristic.h>

#include "CharacteristicBase.h"
#include "CharacteristicBuilder.h"

using namespace SimpleBLE;

BluetoothUUID Characteristic::uuid() { return internal_->uuid(); }

std::vector<Descriptor> Characteristic::descriptors() { return internal_->descriptors(); }

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

bool Characteristic::can_read() { return internal_->can_read(); }
bool Characteristic::can_write_request() { return internal_->can_write_request(); }
bool Characteristic::can_write_command() { return internal_->can_write_command(); }
bool Characteristic::can_notify() { return internal_->can_notify(); }
bool Characteristic::can_indicate() { return internal_->can_indicate(); }