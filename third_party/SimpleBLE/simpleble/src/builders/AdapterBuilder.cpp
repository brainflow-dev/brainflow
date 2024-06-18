#include "AdapterBuilder.h"

using namespace SimpleBLE;

AdapterBuilder::AdapterBuilder(std::shared_ptr<AdapterBase> internal) { internal_ = internal; }