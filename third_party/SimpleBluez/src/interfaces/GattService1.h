#pragma once

#include "simpledbus/SimpleDBus.h"

#include <string>

class GattService1 : public SimpleDBus::Interfaces::PropertyHandler, public SimpleDBus::Properties {
  private:
    SimpleDBus::Connection* _conn;
    std::string _path;

    std::string _uuid;

    void add_option(std::string option_name, SimpleDBus::Holder value);
    void remove_option(std::string option_name);

  public:
    GattService1(SimpleDBus::Connection* conn, std::string path);
    ~GattService1();

    std::string get_uuid();
};
