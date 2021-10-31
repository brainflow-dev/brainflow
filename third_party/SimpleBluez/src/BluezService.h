#pragma once
#include "simpledbus/SimpleDBus.h"

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "BluezAdapter.h"
#include "BluezAgent.h"

class BluezService : public SimpleDBus::Introspectable {
  private:
    SimpleDBus::Connection conn;
    SimpleDBus::ObjectManager object_manager;

    void add_path(std::string path, SimpleDBus::Holder options);
    void remove_path(std::string path, SimpleDBus::Holder options);

    std::shared_ptr<BluezAgent> agent;
    std::map<std::string, std::shared_ptr<BluezAdapter>> adapters;

    void process_received_signal(SimpleDBus::Message& message);

  public:
    BluezService();
    ~BluezService();

    void init();
    void run_async();

    std::shared_ptr<BluezAdapter> get_first_adapter();
    std::shared_ptr<BluezAdapter> get_adapter(std::string adapter_name);
    std::vector<std::shared_ptr<BluezAdapter>> get_all_adapters();
};
