#include <gtest/gtest.h>

#include <simpledbus/advanced/Proxy.h>

using namespace SimpleDBus;

TEST(ProxyLifetime, Validity) {
    Proxy p = Proxy(nullptr, "", "/");

    // A newly created proxy should always be valid.
    ASSERT_TRUE(p.valid());

    // Should notify that the proxy can be safely deleted, as nothing worth keeping is left
    ASSERT_TRUE(p.path_remove("/", Holder::create_array()));

    // A proxy that has been removed is no longer valid.
    ASSERT_FALSE(p.valid());
}
