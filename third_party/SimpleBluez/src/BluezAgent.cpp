#include "BluezAgent.h"

BluezAgent::BluezAgent(std::string path, SimpleDBus::Holder options) : _path(path) {}

BluezAgent::~BluezAgent() {}

bool BluezAgent::process_received_signal(SimpleDBus::Message& message) {
    if (message.get_path() == _path) {
        return true;
    }
    return false;
}