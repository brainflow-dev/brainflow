#pragma once

namespace SimpleBluez {
namespace Config {

extern bool use_system_bus;

namespace Base {
static void reset_all() { use_system_bus = true; }
}  // namespace Base

}  // namespace Config
}  // namespace SimpleBluez
