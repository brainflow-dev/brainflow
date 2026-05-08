#pragma once

#include <atomic>
#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/interfaces/LEAdvertisement1.h>
#include <simpledbus/interfaces/ObjectManager.h>

namespace SimpleBluez {

class Advertisement : public SimpleDBus::Proxy {
  public:

    Advertisement(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Advertisement() = default;

    bool active();
    void activate();
    void deactivate();

    std::string adv_type();
    void adv_type(const std::string& type);

    std::vector<std::string> service_uuids();
    void service_uuids(const std::vector<std::string>& service_uuids);

    std::map<uint16_t, ByteArray> manufacturer_data();
    void manufacturer_data(const std::map<uint16_t, ByteArray>& manufacturer_data);

    std::map<std::string, ByteArray> service_data();
    void service_data(const std::map<std::string, ByteArray>& service_data);

    std::vector<std::string> solicit_uuids();
    void solicit_uuids(const std::vector<std::string>& solicit_uuids);

    std::map<uint8_t, ByteArray> data();
    void data(const std::map<uint8_t, ByteArray>& data);

    bool discoverable();
    void discoverable(bool discoverable);

    uint16_t discoverable_timeout();
    void discoverable_timeout(uint16_t timeout);

    std::vector<std::string> includes();
    void includes(const std::vector<std::string>& includes);

    std::string local_name();
    void local_name(const std::string& name);

    uint16_t appearance();
    void appearance(uint16_t appearance);

    uint16_t duration();
    void duration(uint16_t duration);

    uint16_t timeout();
    void timeout(uint16_t timeout);

    uint32_t min_interval();
    void min_interval(uint32_t interval);

    uint32_t max_interval();
    void max_interval(uint32_t interval);

    int16_t tx_power();
    void tx_power(int16_t power);

    bool include_tx_power();
    void include_tx_power(bool include);

    // ----- INTERNAL CALLBACKS -----
    void on_registration() override;

  private:
    std::atomic_bool _active = false;

    std::shared_ptr<LEAdvertisement1> le_advertisement1();
    std::shared_ptr<SimpleDBus::Interfaces::ObjectManager> object_manager();
};

}  // namespace SimpleBluez