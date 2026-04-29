#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Message.h>

int main(int argc, char* argv[]) {
    SimpleDBus::Connection conn(DBUS_BUS_SESSION);
    conn.init();

    auto msg = SimpleDBus::Message::create_method_call(
        "org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");

    // For information on these arguments, have a look at the interface definition.
    msg.append_argument(SimpleDBus::Holder::create<std::string>("SimpleDBUS Example"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(SimpleDBus::Holder::create<uint32_t>(42), DBUS_TYPE_UINT32_AS_STRING);
    msg.append_argument(SimpleDBus::Holder::create<std::string>("ubuntu-logo"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(SimpleDBus::Holder::create<std::string>("SimpleDBus Notify Summary"),
                        DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(SimpleDBus::Holder::create<std::string>("This is a SimpleDBus notification."),
                        DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(SimpleDBus::Holder::create<std::vector<SimpleDBus::Holder>>(), "as");
    msg.append_argument(SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>(), "a{sv}");
    msg.append_argument(SimpleDBus::Holder::create<int32_t>(5000), DBUS_TYPE_INT32_AS_STRING);
    conn.send_with_reply_and_block(msg);

    return 0;
}
