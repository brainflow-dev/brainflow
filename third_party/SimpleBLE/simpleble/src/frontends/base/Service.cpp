#include <simpleble/Service.h>

#include "BuildVec.h"
#include "ServiceBase.h"

using namespace SimpleBLE;

bool Service::initialized() const { return internal_ != nullptr; }

ServiceBase* Service::operator->() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

const ServiceBase* Service::operator->() const {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_.get();
}

BluetoothUUID Service::uuid() { return (*this)->uuid(); }

ByteArray Service::data() { return (*this)->data(); }

std::vector<Characteristic> Service::characteristics() { return Factory::vector((*this)->characteristics()); }
