#include "DescriptorBuilder.h"

#include "DescriptorBase.h"

using namespace SimpleBLE;

DescriptorBuilder::DescriptorBuilder(const BluetoothUUID& uuid) { internal_ = std::make_shared<DescriptorBase>(uuid); }