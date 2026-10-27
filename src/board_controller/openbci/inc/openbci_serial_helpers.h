#pragma once

#include "serial.h"
#include <chrono>

// Internal board policy; USB identities are deliberately not public board-description fields.
std::vector<SerialPortInfo> get_openbci_serial_candidates (
    int board_id, const std::vector<SerialPortInfo> &ports, const std::string &serial_number);

bool wait_for_openbci_welcome (
    Serial &serial, std::chrono::milliseconds timeout, std::string &response);
