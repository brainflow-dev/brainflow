#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "simpleble/Characteristic.h"

namespace py = pybind11;

constexpr auto kDocsCharacteristic = R"pbdoc(
    Characteristic
)pbdoc";

constexpr auto kDocsCharacteristicUuid = R"pbdoc(
    UUID of the characteristic
)pbdoc";

constexpr auto kDocsCharacteristicDescriptors = R"pbdoc(
    Descriptors of the characteristic
)pbdoc";

constexpr auto kDocsCharacteristicCapabilities = R"pbdoc(
    Capabilities of the characteristic
)pbdoc";

constexpr auto kDocsCharacteristicCanRead = R"pbdoc(
    Whether the characteristic can be read
)pbdoc";

constexpr auto kDocsCharacteristicCanWriteRequest = R"pbdoc(
    Whether the characteristic can be written with a request
)pbdoc";

constexpr auto kDocsCharacteristicCanWriteCommand = R"pbdoc(
    Whether the characteristic can be written with a command
)pbdoc";

constexpr auto kDocsCharacteristicCanNotify = R"pbdoc(
    Whether the characteristic can be notified
)pbdoc";

constexpr auto kDocsCharacteristicCanIndicate = R"pbdoc(
    Whether the characteristic can be indicated
)pbdoc";

void wrap_characteristic(py::module& m) {
    // TODO: Add __str__ and __repr__ methods
    py::class_<SimpleBLE::Characteristic>(m, "Characteristic", kDocsCharacteristic)
        .def("uuid", &SimpleBLE::Characteristic::uuid, kDocsCharacteristicUuid)
        .def("descriptors", &SimpleBLE::Characteristic::descriptors, kDocsCharacteristicDescriptors)
        .def("capabilities", &SimpleBLE::Characteristic::capabilities, kDocsCharacteristicCapabilities)
        .def("can_read", &SimpleBLE::Characteristic::can_read, kDocsCharacteristicCanRead)
        .def("can_write_request", &SimpleBLE::Characteristic::can_write_request, kDocsCharacteristicCanWriteRequest)
        .def("can_write_command", &SimpleBLE::Characteristic::can_write_command, kDocsCharacteristicCanWriteCommand)
        .def("can_notify", &SimpleBLE::Characteristic::can_notify, kDocsCharacteristicCanNotify)
        .def("can_indicate", &SimpleBLE::Characteristic::can_indicate, kDocsCharacteristicCanIndicate);
}
