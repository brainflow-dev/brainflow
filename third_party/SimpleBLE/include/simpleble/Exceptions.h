#pragma once

#include <stdexcept>
#include "Types.h"

namespace SimpleBLE {

namespace Exception {

class InvalidReference : public std::runtime_error {
  public:
    InvalidReference();
};

class ServiceNotFound : public std::runtime_error {
  public:
    ServiceNotFound(BluetoothUUID uuid);
};

class CharacteristicNotFound : public std::runtime_error {
  public:
    CharacteristicNotFound(BluetoothUUID uuid);
};

class OperationNotSupported : public std::runtime_error {
  public:
    OperationNotSupported();
};

class OperationFailed : public std::runtime_error {
  public:
    OperationFailed();
};

}  // namespace Exception

}  // namespace SimpleBLE
