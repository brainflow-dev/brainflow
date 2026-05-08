#pragma once
#include <simpleble/SimpleBLE.h>
#include "org/simplejavable/PeripheralCallback.h"

class PeripheralWrapper {
  public:
    explicit PeripheralWrapper(const SimpleBLE::Peripheral& peripheral);

    SimpleBLE::Peripheral& get();
    const SimpleBLE::Peripheral& get() const;

    void setCallback(Org::SimpleJavaBLE::PeripheralCallback& callback);

    size_t getHash();

  private:
    SimpleBLE::Peripheral _peripheral;
    Org::SimpleJavaBLE::PeripheralCallback _callback;
};