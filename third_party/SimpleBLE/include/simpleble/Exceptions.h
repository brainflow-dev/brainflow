#pragma once

#include <stdexcept>
#include <string>
#include "Types.h"

namespace SimpleBLE {

namespace Exception {

class BaseException : public std::runtime_error {
  public:
    BaseException(const std::string& __arg) : std::runtime_error(__arg) {}
};

class InvalidReference : public BaseException {
  public:
    InvalidReference();
};

class ServiceNotFound : public BaseException {
  public:
    ServiceNotFound(BluetoothUUID uuid);
};

class CharacteristicNotFound : public BaseException {
  public:
    CharacteristicNotFound(BluetoothUUID uuid);
};

class OperationNotSupported : public BaseException {
  public:
    OperationNotSupported();
};

class OperationFailed : public BaseException {
  public:
    OperationFailed();
};

}  // namespace Exception

}  // namespace SimpleBLE
