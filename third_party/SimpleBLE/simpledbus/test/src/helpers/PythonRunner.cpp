#include "PythonRunner.h"
#include <Python.h>
#include <mutex>

#include <csignal>

PythonRunner::PythonRunner(const std::string& script_path) : _script_path(script_path) {
    // In order to get Python to properly find relative imports, the current location
    // of the executable needs to be added to the Python path.
    _import_path = std::filesystem::canonical("/proc/self/exe").parent_path();
}

PythonRunner::~PythonRunner() { uninit(); }

void PythonRunner::init() {
    _async_thread = new std::thread(&PythonRunner::_async_thread_function, this);
    while (!_python_initialized) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void PythonRunner::uninit() {
    if (Py_IsInitialized()) {
        // This is a hack to get the Python interpreter to exit cleanly.
        raise(SIGINT);
    }
    if (_async_thread) {
        while (!_async_thread->joinable()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        _async_thread->join();
        delete _async_thread;
        _async_thread = nullptr;
        _python_initialized = false;
    }
}

void PythonRunner::_async_thread_function() {
    Py_InitializeEx(0);

    // Append the required import path to the Python path.
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(("sys.path.append('" + _import_path.string() + "')").c_str());
    _python_initialized = true;

    // Run the script.
    std::string full_script_path = (_import_path / _script_path).string();
    FILE* fp = fopen(full_script_path.c_str(), "r");
    PyRun_SimpleFile(fp, full_script_path.c_str());
    fclose(fp);
    Py_Finalize();
}
