#pragma once

#include "simpledbus/SimpleDBus.h"

#include <string>

class BluezAgent {
  private:
    std::string _path;

  public:
    BluezAgent(std::string path, SimpleDBus::Holder options);
    ~BluezAgent();

    bool process_received_signal(SimpleDBus::Message& message);
};
