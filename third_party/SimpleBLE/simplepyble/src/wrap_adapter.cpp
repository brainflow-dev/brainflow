#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "simpleble/SimpleBLE.h"

namespace py = pybind11;

constexpr auto kDocsAdapter = R"pbdoc(
    Adapter
)pbdoc";

constexpr auto kDocsAdapterBluetoothEnabled = R"pbdoc(
    Whether Bluetooth is enabled
)pbdoc";

constexpr auto kDocsAdapterGetAdapters = R"pbdoc(
    Get all available adapters
)pbdoc";

constexpr auto kDocsAdapterInitialized = R"pbdoc(
    Whether the adapter is initialized
)pbdoc";

constexpr auto kDocsAdapterIdentifier = R"pbdoc(
    Identifier of the adapter
)pbdoc";

constexpr auto kDocsAdapterAddress = R"pbdoc(
    Address of the adapter
)pbdoc";

constexpr auto kDocsAdapterScanStart = R"pbdoc(
    Start scanning for peripherals
)pbdoc";

constexpr auto kDocsAdapterScanStop = R"pbdoc(
    Stop scanning for peripherals
)pbdoc";

constexpr auto kDocsAdapterScanIsActive = R"pbdoc(
    Whether the adapter is scanning
)pbdoc";

constexpr auto kDocsAdapterScanFor = R"pbdoc(
    Scan for peripherals for a given duration
)pbdoc";

constexpr auto kDocsAdapterScanGetResults = R"pbdoc(
    Get the results of the last scan
)pbdoc";

constexpr auto kDocsAdapterSetCallbackOnScanStart = R"pbdoc(
    Set the callback to be called when scanning starts
)pbdoc";

constexpr auto kDocsAdapterSetCallbackOnScanStop = R"pbdoc(
    Set the callback to be called when scanning stops
)pbdoc";

constexpr auto kDocsAdapterSetCallbackOnScanFound = R"pbdoc(
    Set the callback to be called when a peripheral is found
)pbdoc";

constexpr auto kDocsAdapterSetCallbackOnScanUpdated = R"pbdoc(
    Set the callback to be called when a peripheral is updated
)pbdoc";

constexpr auto kDocsAdapterGetPairedPeripherals = R"pbdoc(
    Get all paired peripherals
)pbdoc";

void wrap_adapter(py::module& m) {
    // TODO: Add __str__ and __repr__ methods
    py::class_<SimpleBLE::Adapter>(m, "Adapter")
        .def("bluetooth_enabled", &SimpleBLE::Adapter::bluetooth_enabled, kDocsAdapterBluetoothEnabled)
        .def("get_adapters", &SimpleBLE::Adapter::get_adapters, kDocsAdapterGetAdapters)
        .def("initialized", &SimpleBLE::Adapter::initialized, kDocsAdapterInitialized)
        .def("identifier", &SimpleBLE::Adapter::identifier, kDocsAdapterIdentifier)
        .def("address", &SimpleBLE::Adapter::address, kDocsAdapterAddress)
        .def("scan_start", &SimpleBLE::Adapter::scan_start, kDocsAdapterScanStart)
        .def("scan_stop", &SimpleBLE::Adapter::scan_stop, kDocsAdapterScanStop)
        .def("scan_is_active", &SimpleBLE::Adapter::scan_is_active, kDocsAdapterScanIsActive)
        .def("scan_for", &SimpleBLE::Adapter::scan_for, py::call_guard<py::gil_scoped_release>(), kDocsAdapterScanFor)
        .def("scan_get_results", &SimpleBLE::Adapter::scan_get_results, kDocsAdapterScanGetResults)
        .def("set_callback_on_scan_start", &SimpleBLE::Adapter::set_callback_on_scan_start, py::keep_alive<1, 2>(), kDocsAdapterSetCallbackOnScanStart)
        .def("set_callback_on_scan_stop", &SimpleBLE::Adapter::set_callback_on_scan_stop, py::keep_alive<1, 2>(), kDocsAdapterSetCallbackOnScanStop)
        .def("set_callback_on_scan_found", &SimpleBLE::Adapter::set_callback_on_scan_found, py::keep_alive<1, 2>(), kDocsAdapterSetCallbackOnScanFound)
        .def("set_callback_on_scan_updated", &SimpleBLE::Adapter::set_callback_on_scan_updated, py::keep_alive<1, 2>(), kDocsAdapterSetCallbackOnScanUpdated)
        .def("get_paired_peripherals", &SimpleBLE::Adapter::get_paired_peripherals, kDocsAdapterGetPairedPeripherals);
}
