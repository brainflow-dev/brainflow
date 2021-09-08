#include <simpleble/Exceptions.h>

using namespace SimpleBLE;
using namespace SimpleBLE::Exception;

InvalidReference::InvalidReference() : BaseException("Underlying reference to object is invalid") {}

ServiceNotFound::ServiceNotFound(BluetoothUUID uuid) : BaseException("Service with UUID " + uuid + " not found") {}

CharacteristicNotFound::CharacteristicNotFound(BluetoothUUID uuid)
    : BaseException("Characteristic with UUID " + uuid + " not found") {}

OperationNotSupported::OperationNotSupported() : BaseException("The requested operation is not supported.") {}

OperationFailed::OperationFailed() : BaseException("The requested operation has failed.") {}
