#include <simpledbus/common/Introspectable.h>
#include <simpledbus/base/Logger.h>

#include <iostream>

using namespace SimpleDBus;

Introspectable::Introspectable(Connection* conn, std::string service, std::string path)
    : _conn(conn), _service(service), _path(path), _interface("org.freedesktop.DBus.Introspectable") {}

Introspectable::~Introspectable() {}

// Names are made matching the ones from the DBus specification
Holder Introspectable::Introspect() {
    LOG_F(DEBUG, "%s -> Introspect()", _path.c_str());
    Message query_msg = Message::create_method_call(_service, _path, _interface, "Introspect");

    Message reply_msg = _conn->send_with_reply_and_block(query_msg);
    Holder result = reply_msg.extract();
    return result;
}
