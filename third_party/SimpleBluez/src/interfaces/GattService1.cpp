#include "GattService1.h"

#include <iostream>

GattService1::GattService1(SimpleDBus::Connection* conn, std::string path) : _conn(conn), _path(path), Properties{conn, "org.bluez", path}, PropertyHandler(path) {
    // std::cout << "Creating org.bluez.GattService1: " << path << std::endl;
}

GattService1::~GattService1() {}

void GattService1::add_option(std::string option_name, SimpleDBus::Holder value) {
    if (option_name == "UUID") {
        _uuid = value.get_string();
    }
}

void GattService1::remove_option(std::string option_name) {}

std::string GattService1::get_uuid() { return _uuid; }
