#include <pybind11/pybind11.h>
#include "simpleble/Config.h"

namespace py = pybind11;

// Wrapper classes for Python bindings (not exposed to C++)
namespace PyWrappers {
    struct WinRT {
        static bool get_experimental_use_own_mta_apartment() {
            return SimpleBLE::Config::WinRT::experimental_use_own_mta_apartment;
        }
        static void set_experimental_use_own_mta_apartment(bool value) {
            SimpleBLE::Config::WinRT::experimental_use_own_mta_apartment = value;
        }
        static bool get_experimental_reinitialize_winrt_apartment_on_main_thread() {
            return SimpleBLE::Config::WinRT::experimental_reinitialize_winrt_apartment_on_main_thread;
        }
        static void set_experimental_reinitialize_winrt_apartment_on_main_thread(bool value) {
            SimpleBLE::Config::WinRT::experimental_reinitialize_winrt_apartment_on_main_thread = value;
        }

        static void reset() {
            SimpleBLE::Config::WinRT::reset();
        }
    };

    struct SimpleBluez {
        static void reset() {
            SimpleBLE::Config::SimpleBluez::reset();
        }
    };

    struct CoreBluetooth {
        static void reset() {
            SimpleBLE::Config::CoreBluetooth::reset();
        }
    };

    struct Android {
        static void reset() {
            SimpleBLE::Config::Android::reset();
        }
    };

    struct Base {
        static void reset_all() {
            SimpleBLE::Config::Base::reset_all();
        }
    };
}

// Documentation strings
constexpr auto kDocsConfigModule = R"pbdoc(
    Configuration options for SimpleBLE
)pbdoc";

constexpr auto kDocsConfigWinRTClass = R"pbdoc(
    WinRT-specific configuration options
)pbdoc";

constexpr auto kDocsConfigSimpleBluezClass = R"pbdoc(
    SimpleBluez-specific configuration options
)pbdoc";

constexpr auto kDocsConfigCoreBluetoothClass = R"pbdoc(
    CoreBluetooth-specific configuration options
)pbdoc";

constexpr auto kDocsConfigAndroidClass = R"pbdoc(
    Android-specific configuration options
)pbdoc";

constexpr auto kDocsConfigBaseClass = R"pbdoc(
    Base configuration options
)pbdoc";

constexpr auto kDocsConfigWinRTExperimentalMTA = R"pbdoc(
    Use own MTA apartment (experimental)
)pbdoc";

constexpr auto kDocsConfigWinRTExperimentalReinitializeMTAOnMainThread = R"pbdoc(
    Reinitialize the WinRT apartment on the main thread (experimental)
)pbdoc";

constexpr auto kDocsConfigWinRTReset = R"pbdoc(
    Reset WinRT configuration options to their default values
)pbdoc";

constexpr auto kDocsConfigSimpleBluezReset = R"pbdoc(
    Reset SimpleBluez configuration options to their default values
)pbdoc";

constexpr auto kDocsConfigCoreBluetoothReset = R"pbdoc(
    Reset CoreBluetooth configuration options to their default values
)pbdoc";

constexpr auto kDocsConfigAndroidReset = R"pbdoc(
    Reset Android configuration options to their default values
)pbdoc";

constexpr auto kDocsConfigBaseResetAll = R"pbdoc(
    Reset all configuration options to their default values
)pbdoc";

void wrap_config(py::module& m) {
    auto config = m.def_submodule("config", kDocsConfigModule);

    // Define classes directly under config
    py::class_<PyWrappers::WinRT> winrt_config(config, "winrt", kDocsConfigWinRTClass, py::metaclass());
    winrt_config
        .def_property_static("experimental_use_own_mta_apartment",
            [](py::object) { return PyWrappers::WinRT::get_experimental_use_own_mta_apartment(); },
            [](py::object, bool value) { PyWrappers::WinRT::set_experimental_use_own_mta_apartment(value); },
            kDocsConfigWinRTExperimentalMTA)
        .def_property_static("experimental_reinitialize_winrt_apartment_on_main_thread",
            [](py::object) { return PyWrappers::WinRT::get_experimental_reinitialize_winrt_apartment_on_main_thread(); },
            [](py::object, bool value) { PyWrappers::WinRT::set_experimental_reinitialize_winrt_apartment_on_main_thread(value); },
            kDocsConfigWinRTExperimentalReinitializeMTAOnMainThread)
        .def_static("reset", &PyWrappers::WinRT::reset, kDocsConfigWinRTReset);

    py::class_<PyWrappers::SimpleBluez> simplebluez_config(config, "simplebluez", kDocsConfigSimpleBluezClass, py::metaclass());
    simplebluez_config
        .def_static("reset", &PyWrappers::SimpleBluez::reset, kDocsConfigSimpleBluezReset);

    py::class_<PyWrappers::CoreBluetooth> corebluetooth_config(config, "corebluetooth", kDocsConfigCoreBluetoothClass, py::metaclass());
    corebluetooth_config
        .def_static("reset", &PyWrappers::CoreBluetooth::reset, kDocsConfigCoreBluetoothReset);

    py::class_<PyWrappers::Android> android_config(config, "android", kDocsConfigAndroidClass, py::metaclass());
    android_config
        .def_static("reset", &PyWrappers::Android::reset, kDocsConfigAndroidReset);

    py::class_<PyWrappers::Base> base_config(config, "base", kDocsConfigBaseClass, py::metaclass());
    base_config
        .def_static("reset_all", &PyWrappers::Base::reset_all, kDocsConfigBaseResetAll);
}