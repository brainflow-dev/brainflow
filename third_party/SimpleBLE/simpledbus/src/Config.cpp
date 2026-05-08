#include <simpledbus/Config.h>

namespace SimpleDBus {
namespace Config {

namespace Connection {
std::chrono::steady_clock::duration send_with_reply_timeout = std::chrono::seconds(30);
}  // namespace Connection

}  // namespace Config
}  // namespace SimpleDBus
