#include <gtest/gtest.h>

#include <simpledbus/advanced/InterfaceRegistry.h>
#include <simpledbus/advanced/Proxy.h>

using namespace SimpleDBus;

const AutoRegisterInterface<Interface> registry_i1{
    "i.1",
    // clang-format off
    [](std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::make_shared<Interface>(conn, proxy, "i.1");
    }
    // clang-format on
};

const AutoRegisterInterface<Interface> registry_i2{
    "i.2",
    // clang-format off
    [](std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::make_shared<Interface>(conn, proxy, "i.2");
    }
    // clang-format on
};

const AutoRegisterInterface<Interface> registry_i3{
    "i.3",
    // clang-format off
    [](std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::make_shared<Interface>(conn, proxy, "i.3");
    }
    // clang-format on
};

TEST(ProxyInterfaces, LoadInterfaces) {
    Holder managed_interfaces = Holder::create<std::map<std::string, Holder>>();
    managed_interfaces.dict_append(Holder::STRING, "i.1", Holder());

    auto h = std::make_shared<Proxy>(nullptr, "", "/");
    EXPECT_FALSE(h->interfaces_loaded());

    // TODO: Check that all properties of the interface are correctly loaded.

    h->interfaces_load(managed_interfaces);

    EXPECT_TRUE(h->interfaces_loaded());
    EXPECT_EQ(1, h->interfaces().size());
    EXPECT_EQ(1, h->interfaces().count("i.1"));
}

TEST(ProxyInterfaces, UnloadInterfaces) {
    Holder managed_interfaces = Holder::create<std::map<std::string, Holder>>();
    managed_interfaces.dict_append(Holder::STRING, "i.1", Holder());
    managed_interfaces.dict_append(Holder::STRING, "i.2", Holder());
    managed_interfaces.dict_append(Holder::STRING, "i.3", Holder());

    auto h = std::make_shared<Proxy>(nullptr, "", "/");
    h->interfaces_load(managed_interfaces);
    EXPECT_EQ(3, h->interfaces_count());

    Holder removed_interfaces = Holder::create<std::vector<Holder>>();
    removed_interfaces.array_append(Holder::create<std::string>("i.3"));
    h->interfaces_unload(removed_interfaces);

    EXPECT_EQ(2, h->interfaces_count());
    EXPECT_TRUE(h->interfaces_loaded());
    EXPECT_FALSE(h->interfaces().at("i.3")->is_loaded());

    removed_interfaces = Holder::create<std::vector<Holder>>();
    removed_interfaces.array_append(Holder::create<std::string>("i.2"));
    h->interfaces_unload(removed_interfaces);

    EXPECT_EQ(1, h->interfaces_count());
    EXPECT_TRUE(h->interfaces_loaded());
    EXPECT_FALSE(h->interfaces().at("i.2")->is_loaded());

    removed_interfaces = Holder::create<std::vector<Holder>>();
    removed_interfaces.array_append(Holder::create<std::string>("i.1"));
    h->interfaces_unload(removed_interfaces);

    EXPECT_EQ(0, h->interfaces_count());
    EXPECT_FALSE(h->interfaces_loaded());
    EXPECT_FALSE(h->interfaces().at("i.1")->is_loaded());
}

TEST(ProxyInterfaces, ReloadInterfaces) {
    Holder managed_interfaces = Holder::create<std::map<std::string, Holder>>();
    managed_interfaces.dict_append(Holder::STRING, "i.1", Holder());
    managed_interfaces.dict_append(Holder::STRING, "i.2", Holder());
    managed_interfaces.dict_append(Holder::STRING, "i.3", Holder());

    auto h = std::make_shared<Proxy>(nullptr, "", "/");
    h->interfaces_load(managed_interfaces);
    EXPECT_EQ(3, h->interfaces_count());

    managed_interfaces = Holder::create<std::map<std::string, Holder>>();
    managed_interfaces.dict_append(Holder::STRING, "i.1", Holder());
    managed_interfaces.dict_append(Holder::STRING, "i.3", Holder());
    h->interfaces_reload(managed_interfaces);

    EXPECT_EQ(2, h->interfaces_count());
}
