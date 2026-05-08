#pragma once
#include <chrono>

namespace SimpleDBus {
namespace Config {

namespace Connection {
extern std::chrono::steady_clock::duration send_with_reply_timeout;

static void reset() { send_with_reply_timeout = std::chrono::seconds(30); }
}  // namespace Connection

namespace Base {
static void reset_all() { Connection::reset(); }
}  // namespace Base

}  // namespace Config
}  // namespace SimpleDBus
