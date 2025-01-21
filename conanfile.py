from conans import ConanFile, CMake


class BrainflowConan(ConanFile):
    name = "brainflow"
    version = "5.2.0"

    # Optional metadata
    license = "MIT"
    author = "Andrey1994 andrey@brainflow.org"
    url = "https://github.com/brainflow-dev/brainflow"
    description = "BrainFlow is a library intended to obtain, parse and analyze EEG, EMG, ECG and other kinds of data from biosensors"
    topics = ("eeg", "bci", "neurotech")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"libftdi": [True, False], "openmp": [True, False], "onnx": [True, False], "bluetooth": [True, False],
               "ble": [True, False], "periphery": [True, False], "oymotion": [True, False],"synchroni": [True, False],
               "static_msvc_runtime": [True, False]}
    default_options = {"libftdi": False, "openmp": False, "onnx": True, "bluetooth": True,
                       "ble": True, "periphery": False, "oymotion": False,"synchroni": True, "static_msvc_runtime": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "third_party/*", "cpp_package/build.cmake", "cpp_package/src/*", "cmake/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.libftdi
            del self.options.periphery
        else:
            del self.options.oymotion
            del self.options.msvc_runtime

    def build(self):
        cmake = CMake(self)
        if self.settings.os != "Windows" and self.options.libftdi:
            cmake.definitions["USE_LIBFTDI"] = "ON"
        if self.options.openmp:
            cmake.definitions["USE_OPENMP"] = "ON"
        if self.options.onnx:
            cmake.definitions["BUILD_ONNX"] = "ON"
        if self.options.bluetooth:
            cmake.definitions["BUILD_BLUETOOTH"] = "ON"
        if self.options.ble:
            cmake.definitions["BUILD_BLE"] = "ON"
        if self.options.synchroni:
            if self.settings.os == "Android":
                cmake.definitions["BUILD_SYNCHRONI_SDK"] = "OFF"
            else:
                cmake.definitions["BUILD_SYNCHRONI_SDK"] = "ON"
        if self.settings.os != "Windows" and self.options.periphery:
            cmake.definitions["USE_PERIPHERY"] = "ON"
        if self.settings.os == "Windows" and self.options.oymotion:
            cmake.definitions["BUILD_OYMOTION_SDK"] = "ON"
        if self.settings.os == "Windows" and self.options.static_msvc_runtime:
            cmake.definitions["MSVC_RUNTIME"] = "static"
        else:
            cmake.definitions["MSVC_RUNTIME"] = "dynamic"
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["brainflow"]
