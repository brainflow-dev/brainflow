#include <simplebluez/Bluez.h>
#include <simpledbus/interfaces/ObjectManager.h>

#include <simplebluez/Config.h>

using namespace SimpleBluez;

Bluez::Bluez()
    : _conn(std::make_shared<SimpleDBus::Connection>(Config::use_system_bus ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION)) {}

Bluez::~Bluez() {
    if (_conn->is_initialized()) {
        _conn->remove_match("type='signal',sender='org.bluez'");
    }
}

void Bluez::init() {
    _conn->init();
    _conn->add_match("type='signal',sender='org.bluez'");

    _bluez_root = SimpleDBus::Proxy::create<BluezRoot>(_conn, "org.bluez", "/");
    _bluez_root->load_managed_objects();

    _custom_root = SimpleDBus::Proxy::create<CustomRoot>(_conn, "org.simplebluez", "/");
}

void Bluez::run_async() { _conn->read_write_dispatch(); }

std::shared_ptr<CustomRoot> Bluez::root_custom() { return _custom_root; }

std::shared_ptr<BluezRoot> Bluez::root_bluez() { return _bluez_root; }

std::vector<std::shared_ptr<Adapter>> Bluez::get_adapters() { return _bluez_root->get_adapters(); }

void Bluez::register_agent(std::shared_ptr<Agent> agent) { _bluez_root->register_agent(agent); }
