#include "AdapterBase.h"

namespace SimpleBLE {

void AdapterBase::set_callback_on_power_on(std::function<void()> on_power_on) {
    if (on_power_on) {
        _callback_on_power_on.load(on_power_on);
    } else {
        _callback_on_power_on.unload();
    }
}

void AdapterBase::set_callback_on_power_off(std::function<void()> on_power_off) {
    if (on_power_off) {
        _callback_on_power_off.load(on_power_off);
    } else {
        _callback_on_power_off.unload();
    }
}

void AdapterBase::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    if (on_scan_start) {
        _callback_on_scan_start.load(on_scan_start);
    } else {
        _callback_on_scan_start.unload();
    }
}

void AdapterBase::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    if (on_scan_stop) {
        _callback_on_scan_stop.load(on_scan_stop);
    } else {
        _callback_on_scan_stop.unload();
    }
}

void AdapterBase::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    if (on_scan_updated) {
        _callback_on_scan_updated.load(on_scan_updated);
    } else {
        _callback_on_scan_updated.unload();
    }
}

void AdapterBase::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    if (on_scan_found) {
        _callback_on_scan_found.load(on_scan_found);
    } else {
        _callback_on_scan_found.unload();
    }
}

}
