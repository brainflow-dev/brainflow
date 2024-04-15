#pragma once

#include <stdexcept>
#include <string>

#include <simpleble/export.h>

#include "Types.h"

namespace SimpleBLE {

namespace Exception {

class SIMPLEBLE_EXPORT BaseException : public std::runtime_error {
  public:
    BaseException(const std::string& __arg) : std::runtime_error(__arg) {}
};

class SIMPLEBLE_EXPORT NotInitialized : public BaseException {
  public:
    NotInitialized();
};

class SIMPLEBLE_EXPORT NotConnected : public BaseException {
  public:
    NotConnected();
};

class SIMPLEBLE_EXPORT InvalidReference : public BaseException {
  public:
    InvalidReference();
};

class SIMPLEBLE_EXPORT ServiceNotFound : public BaseException {
  public:
    ServiceNotFound(BluetoothUUID uuid);
};

class SIMPLEBLE_EXPORT CharacteristicNotFound : public BaseException {
  public:
    CharacteristicNotFound(BluetoothUUID uuid);
};

class SIMPLEBLE_EXPORT DescriptorNotFound : public BaseException {
  public:
    DescriptorNotFound(BluetoothUUID uuid);
};

class SIMPLEBLE_EXPORT OperationNotSupported : public BaseException {
  public:
    OperationNotSupported();
};

class SIMPLEBLE_EXPORT OperationFailed : public BaseException {
  public:
    OperationFailed();
    OperationFailed(const std::string& err_msg);
};

class SIMPLEBLE_EXPORT WinRTException : public BaseException {
  public:
    WinRTException(int32_t err_code, const std::string& err_msg);
};

class SIMPLEBLE_EXPORT CoreBluetoothException : public BaseException {
  public:
    CoreBluetoothException(const std::string& err_msg);
};

}  // namespace Exception

}  // namespace SimpleBLE
