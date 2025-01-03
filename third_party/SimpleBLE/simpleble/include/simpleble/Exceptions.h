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

class NotInitialized : public BaseException {
  public:
    NotInitialized();
};

class NotConnected : public BaseException {
  public:
    NotConnected();
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

class DescriptorNotFound : public BaseException {
  public:
    DescriptorNotFound(BluetoothUUID uuid);
};

class OperationNotSupported : public BaseException {
  public:
    OperationNotSupported();
};

class OperationFailed : public BaseException {
  public:
    OperationFailed();
    OperationFailed(const std::string& err_msg);
};

class WinRTException : public BaseException {
  public:
    WinRTException(int32_t err_code, const std::string& err_msg);
};

class CoreBluetoothException : public BaseException {
  public:
    CoreBluetoothException(const std::string& err_msg);
};

}  // namespace Exception

}  // namespace SimpleBLE
