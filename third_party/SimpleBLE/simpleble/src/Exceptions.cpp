#include <fmt/core.h>
#include <simpleble/Exceptions.h>

using namespace SimpleBLE;
using namespace SimpleBLE::Exception;

NotInitialized::NotInitialized() : BaseException("Object has not been initialized.") {}

NotConnected::NotConnected() : BaseException("Peripheral is not connected.") {}

InvalidReference::InvalidReference() : BaseException("Underlying reference to object is invalid.") {}

ServiceNotFound::ServiceNotFound(BluetoothUUID uuid) : BaseException("Service with UUID " + uuid + " not found.") {}

CharacteristicNotFound::CharacteristicNotFound(BluetoothUUID uuid)
    : BaseException("Characteristic with UUID " + uuid + " not found") {}

DescriptorNotFound::DescriptorNotFound(BluetoothUUID uuid)
    : BaseException("Descriptor with UUID " + uuid + " not found") {}

OperationNotSupported::OperationNotSupported() : BaseException("The requested operation is not supported.") {}

OperationFailed::OperationFailed() : BaseException("The requested operation has failed.") {}

OperationFailed::OperationFailed(const std::string& err_msg) : BaseException("Operation Failed: " + err_msg) {}

WinRTException::WinRTException(int32_t err_code, const std::string& err_msg)
    : BaseException(fmt::format("WinRT Exception. Error code {}: {}", err_code, err_msg)) {}

CoreBluetoothException::CoreBluetoothException(const std::string& err_msg)
    : BaseException(fmt::format("CoreBluetooth Exception: {}", err_msg)) {}
