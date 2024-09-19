#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "simpleble/Service.h"

namespace py = pybind11;

constexpr auto kDocsService = R"pbdoc(
    Service
)pbdoc";

constexpr auto kDocsServiceUuid = R"pbdoc(
    UUID of the service
)pbdoc";

constexpr auto kDocsServiceData = R"pbdoc(
    Advertised service data
)pbdoc";

constexpr auto kDocsServiceCharacteristics = R"pbdoc(
    Characteristics of the service
)pbdoc";

void wrap_service(py::module& m) {
    // TODO: Add __str__ and __repr__ methods
    py::class_<SimpleBLE::Service>(m, "Service", kDocsService)
        .def("uuid", &SimpleBLE::Service::uuid, kDocsServiceUuid)
        .def(
            "data", [](SimpleBLE::Service& s) { return py::bytes(s.data()); }, kDocsServiceData)
        .def("characteristics", &SimpleBLE::Service::characteristics, kDocsServiceCharacteristics);
}
