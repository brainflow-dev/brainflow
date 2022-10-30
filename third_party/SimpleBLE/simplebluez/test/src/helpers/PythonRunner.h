#pragma once

#include <atomic>
#include <filesystem>
#include <string>
#include <thread>

class PythonRunner {
  public:
    PythonRunner(const std::string& script_path);
    ~PythonRunner();
    PythonRunner(PythonRunner& other) = delete;    // Remove the copy constructor
    void operator=(const PythonRunner&) = delete;  // Remove the copy assignment

    void init();
    void uninit();

  private:
    std::atomic_bool _python_initialized = {false};
    std::filesystem::path _import_path;
    std::string _script_path;
    std::thread* _async_thread;
    void _async_thread_function();
};
