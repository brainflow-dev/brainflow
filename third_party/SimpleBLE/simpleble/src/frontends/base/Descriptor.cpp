#include <simpleble/Descriptor.h>

#include "DescriptorBase.h"
#include "DescriptorBuilder.h"

using namespace SimpleBLE;

BluetoothUUID Descriptor::uuid() { return internal_->uuid(); }