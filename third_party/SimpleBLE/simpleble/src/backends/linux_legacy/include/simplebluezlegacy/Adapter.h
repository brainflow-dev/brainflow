#pragma once

#include <simpledbuslegacy/advanced/Proxy.h>

#include <simplebluezlegacy/Device.h>
#include <simplebluezlegacy/interfaces/Adapter1.h>

#include <kvn/kvn_safe_callback.hpp>

#include <functional>

namespace SimpleBluezLegacy {

class Adapter : public SimpleDBusLegacy::Proxy {
  public:
    typedef Adapter1::DiscoveryFilter DiscoveryFilter;

    Adapter(std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Adapter();

    std::string identifier() const;
    std::string address();
    bool discovering();
    bool powered();

    void discovery_filter(const DiscoveryFilter& filter);
    void discovery_start();
    void discovery_stop();

    std::shared_ptr<Device> device_get(const std::string& path);
    void device_remove(const std::string& path);
    void device_remove(const std::shared_ptr<Device>& device);
    std::vector<std::shared_ptr<Device>> device_paired_get();

    void set_on_device_updated(std::function<void(std::shared_ptr<Device> device)> callback);
    void clear_on_device_updated();

  private:
    std::shared_ptr<SimpleDBusLegacy::Proxy> path_create(const std::string& path) override;

    std::shared_ptr<Adapter1> adapter1();

    kvn::safe_callback<void(std::shared_ptr<Device> device)> _on_device_updated;
};

}  // namespace SimpleBluezLegacy
