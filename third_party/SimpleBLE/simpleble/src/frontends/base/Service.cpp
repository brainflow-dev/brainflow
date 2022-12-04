#include <simpleble/Service.h>

#include "ServiceBase.h"
#include "ServiceBuilder.h"

using namespace SimpleBLE;

BluetoothUUID Service::uuid() { return internal_->uuid(); }

std::vector<Characteristic> Service::characteristics() { return internal_->characteristics(); }