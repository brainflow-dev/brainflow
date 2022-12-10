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

void wrap_characteristic(py::module& m) {
    // TODO: Add __str__ and __repr__ methods
    py::class_<SimpleBLE::Characteristic>(m, "Characteristic", kDocsCharacteristic)
        .def("uuid", &SimpleBLE::Characteristic::uuid, kDocsCharacteristicUuid)
        .def("descriptors", &SimpleBLE::Characteristic::descriptors, kDocsCharacteristicDescriptors);
}
