#pragma once
#include <functional>
#include <string>

#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Holder.h>
#include <simpledbus/base/Message.h>

namespace SimpleDBus {

class Holder;
class Connection;

class Properties {
  private:
    const std::string _interface;

    std::string _path;
    std::string _service;
    Connection* _conn;

  public:
    Properties(Connection* conn, std::string service, std::string path);
    virtual ~Properties();

    // Names are made matching the ones from the DBus specification
    Holder Get(std::string interface, std::string name);
    Holder GetAll(std::string interface);
    void Set(std::string interface, std::string name, Holder value);
};

}  // namespace SimpleDBus
