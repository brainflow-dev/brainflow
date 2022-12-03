#include "PeripheralBuilder.h"

using namespace SimpleBLE;

PeripheralBuilder::PeripheralBuilder(std::shared_ptr<PeripheralBase> internal) { internal_ = internal; }