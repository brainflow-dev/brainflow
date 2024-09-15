#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "simpleble/Peripheral.h"

namespace py = pybind11;

constexpr auto kDocsPeripheral = R"pbdoc(
    Peripheral
)pbdoc";

constexpr auto kDocsPeripheralInitialized = R"pbdoc(
    Whether the peripheral is initialized
)pbdoc";

constexpr auto kDocsPeripheralIdentifier = R"pbdoc(
    Identifier of the peripheral
)pbdoc";

constexpr auto kDocsPeripheralAddress = R"pbdoc(
    Address of the peripheral
)pbdoc";

constexpr auto kDocsPeripheralAddressType = R"pbdoc(
    Address Type of the peripheral
)pbdoc";

constexpr auto kDocsPeripheralRSSI = R"pbdoc(
    RSSI of the peripheral
)pbdoc";

constexpr auto kDocsPeripheralTxPower = R"pbdoc(
    Transit Power of the peripheral in dBm
)pbdoc";

constexpr auto kDocsPeripheralMtu = R"pbdoc(
    Get the negotiated MTU value
)pbdoc";

constexpr auto kDocsPeripheralConnect = R"pbdoc(
    Connect to the peripheral
)pbdoc";

constexpr auto kDocsPeripheralDisconnect = R"pbdoc(
    Disconnect from the peripheral
)pbdoc";

constexpr auto kDocsPeripheralIsConnected = R"pbdoc(
    Whether the peripheral is connected
)pbdoc";

constexpr auto kDocsPeripheralIsConnectable = R"pbdoc(
    Whether the peripheral is connectable
)pbdoc";

constexpr auto kDocsPeripheralIsPaired = R"pbdoc(
    Whether the peripheral is paired
)pbdoc";

constexpr auto kDocsPeripheralUnpair = R"pbdoc(
    Unpair the peripheral
)pbdoc";

constexpr auto kDocsPeripheralServices = R"pbdoc(
    Services of the peripheral
)pbdoc";

constexpr auto kDocsPeripheralManufacturerData = R"pbdoc(
    Manufacturer data of the peripheral
)pbdoc";

constexpr auto kDocsPeripheralReadCharacteristic = R"pbdoc(
    Read a characteristic from the peripheral
)pbdoc";

constexpr auto kDocsPeripheralWriteRequest = R"pbdoc(
    Write a request to the peripheral
)pbdoc";

constexpr auto kDocsPeripheralWriteCommand = R"pbdoc(
    Write a command to the peripheral
)pbdoc";

constexpr auto kDocsPeripheralNotify = R"pbdoc(
    Notify a characteristic from the peripheral
)pbdoc";

constexpr auto kDocsPeripheralIndicate = R"pbdoc(
    Indicate a characteristic from the peripheral
)pbdoc";

constexpr auto kDocsPeripheralUnsubscribe = R"pbdoc(
    Unsubscribe a characteristic from the peripheral
)pbdoc";

constexpr auto kDocsPeripheralDescriptorRead = R"pbdoc(
    Read a descriptor from the peripheral
)pbdoc";

constexpr auto kDocsPeripheralDescriptorWrite = R"pbdoc(
    Write a descriptor to the peripheral
)pbdoc";

constexpr auto kDocsPeripheralSetCallbackOnConnected = R"pbdoc(
    Set callback on connected
)pbdoc";

constexpr auto kDocsPeripheralSetCallbackOnDisconnected = R"pbdoc(
    Set callback on disconnected
)pbdoc";

// clang-format off

void wrap_peripheral(py::module& m) {
    // TODO: Add __str__ and __repr__ methods
    py::class_<SimpleBLE::Peripheral>(m, "Peripheral", kDocsPeripheral)
        .def("initialized", &SimpleBLE::Peripheral::initialized, kDocsPeripheralInitialized)
        .def("identifier", &SimpleBLE::Peripheral::identifier, kDocsPeripheralIdentifier)
        .def("address", &SimpleBLE::Peripheral::address, kDocsPeripheralAddress)
        .def("address_type", &SimpleBLE::Peripheral::address_type, kDocsPeripheralAddressType)
        .def("rssi", &SimpleBLE::Peripheral::rssi, kDocsPeripheralRSSI)
        .def("tx_power", &SimpleBLE::Peripheral::tx_power, kDocsPeripheralTxPower)
        .def("mtu", &SimpleBLE::Peripheral::mtu, kDocsPeripheralMtu)
        .def("connect", &SimpleBLE::Peripheral::connect, py::call_guard<py::gil_scoped_release>(),
             kDocsPeripheralConnect)
        .def("disconnect", &SimpleBLE::Peripheral::disconnect, kDocsPeripheralDisconnect)
        .def("is_connected", &SimpleBLE::Peripheral::is_connected, kDocsPeripheralIsConnected)
        .def("is_connectable", &SimpleBLE::Peripheral::is_connectable, kDocsPeripheralIsConnectable)
        .def("is_paired", &SimpleBLE::Peripheral::is_paired, kDocsPeripheralIsPaired)
        .def("unpair", &SimpleBLE::Peripheral::unpair, kDocsPeripheralUnpair)
        .def("services", &SimpleBLE::Peripheral::services, kDocsPeripheralServices)
        .def(
            "manufacturer_data",
            [](SimpleBLE::Peripheral& p) {
                std::map<uint16_t, py::bytes> ret;
                for (auto& kv : p.manufacturer_data()) {
                    ret[kv.first] = py::bytes(kv.second);
                }
                return ret;
            },
            kDocsPeripheralManufacturerData)
        .def(
            "read",
            [](SimpleBLE::Peripheral& p, std::string const& service, std::string const& characteristic) {
                return py::bytes(p.read(service, characteristic));
            },
            kDocsPeripheralReadCharacteristic)
        .def(
            "write_request",
            [](SimpleBLE::Peripheral& p, std::string service, std::string characteristic, py::bytes payload) {
                // Note py::bytes implicitly converts to std::string
                SimpleBLE::ByteArray cpp_payload(payload);
                py::gil_scoped_release release;
                p.write_request(service, characteristic, cpp_payload);
            },
            kDocsPeripheralWriteRequest)
        .def(
            "write_command",
            [](SimpleBLE::Peripheral& p, std::string service, std::string characteristic, py::bytes payload) {
                // Note py::bytes implicitly converts to std::string
                SimpleBLE::ByteArray cpp_payload(payload);
                py::gil_scoped_release release;
                p.write_command(service, characteristic, cpp_payload);
            },
            kDocsPeripheralWriteCommand)
        .def(
            "notify",
            [](SimpleBLE::Peripheral& p, std::string service, std::string characteristic, std::function<void(py::bytes payload)> cb) {
                p.notify(service, characteristic, [cb](SimpleBLE::ByteArray payload) {
                    py::gil_scoped_acquire gil;
                    cb(py::bytes(payload));
                });
            },
            kDocsPeripheralNotify)
        .def(
            "indicate",
            [](SimpleBLE::Peripheral& p, std::string service, std::string characteristic, std::function<void(py::bytes payload)> cb) {
                p.indicate(service, characteristic, [cb](SimpleBLE::ByteArray payload) {
                    py::gil_scoped_acquire gil;
                    cb(py::bytes(payload));
                });
            },
            kDocsPeripheralIndicate)
        .def("unsubscribe", &SimpleBLE::Peripheral::unsubscribe, kDocsPeripheralUnsubscribe)

        .def(
            "descriptor_read",
            [](SimpleBLE::Peripheral& p, std::string const& service, std::string const& characteristic, std::string const& descriptor) {
                return py::bytes(p.read(service, characteristic, descriptor));
            },
            kDocsPeripheralDescriptorRead)
        .def(
            "descriptor_write",
            [](SimpleBLE::Peripheral& p, std::string service, std::string characteristic, std::string const& descriptor, py::bytes payload) {
                // Note py::bytes implicitly converts to std::string
                p.write(service, characteristic, descriptor, SimpleBLE::ByteArray(payload));
            },
            kDocsPeripheralDescriptorWrite)

        .def("set_callback_on_connected", &SimpleBLE::Peripheral::set_callback_on_connected, py::keep_alive<1, 2>(),
             kDocsPeripheralSetCallbackOnConnected)
        .def("set_callback_on_disconnected", &SimpleBLE::Peripheral::set_callback_on_disconnected,
             py::keep_alive<1, 2>(), kDocsPeripheralSetCallbackOnDisconnected);
}

// clang-format on