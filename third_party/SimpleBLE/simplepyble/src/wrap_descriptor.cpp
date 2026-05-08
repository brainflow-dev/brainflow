#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "simpleble/Descriptor.h"

namespace py = pybind11;

constexpr auto kDocsDescriptor = R"pbdoc(
    Descriptor
)pbdoc";

constexpr auto kDocsDescriptorInitialized = R"pbdoc(
    Whether the descriptor is initialized
)pbdoc";

constexpr auto kDocsDescriptorUuid = R"pbdoc(
    UUID of the descriptor
)pbdoc";

void wrap_descriptor(py::module& m) {
    // TODO: Add __str__ and __repr__ methods
    py::class_<SimpleBLE::Descriptor>(m, "Descriptor", kDocsDescriptor)
        .def("initialized", &SimpleBLE::Descriptor::initialized, kDocsDescriptorInitialized)
        .def("uuid", &SimpleBLE::Descriptor::uuid, kDocsDescriptorUuid);
}
