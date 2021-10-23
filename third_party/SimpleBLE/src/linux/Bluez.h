#pragma once

#include <atomic>
#include <thread>
#include "BluezService.h"

namespace SimpleBLE {

class Bluez {
  public:
    static Bluez* get();

    BluezService bluez_service;

  private:
    Bluez();
    ~Bluez();
    Bluez(Bluez& other) = delete;           // Remove the copy constructor
    void operator=(const Bluez&) = delete;  // Remove the copy assignment

    std::thread* async_thread;
    std::atomic_bool async_thread_active;
    void async_thread_function();
};

}  // namespace SimpleBLE
