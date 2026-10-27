#include <algorithm>
#include <thread>

#include "brainflow_constants.h"
#include "openbci_serial_helpers.h"

std::vector<SerialPortInfo> get_openbci_serial_candidates (
    int board_id, const std::vector<SerialPortInfo> &ports, const std::string &serial_number)
{
    std::vector<SerialPortInfo> candidates;
    if (board_id != (int)BoardIds::CYTON_BOARD && board_id != (int)BoardIds::CYTON_DAISY_BOARD)
    {
        return candidates;
    }
    // Cyton's FT231X dongle. These are adapter IDs, not proof of a Cyton connection;
    // the normal board handshake must still succeed. Do not add generic FT232 IDs.
    const int cyton_vid = 0x0403;
    const int cyton_pid = 0x6015;
    for (const auto &port : ports)
    {
        if (!port.port.empty () && port.vid == cyton_vid && port.pid == cyton_pid &&
            (serial_number.empty () || port.serial_number == serial_number))
        {
            candidates.push_back (port);
        }
    }
    std::sort (candidates.begin (), candidates.end (),
        [] (const SerialPortInfo &a, const SerialPortInfo &b) { return a.port < b.port; });
    candidates.erase (
        std::unique (candidates.begin (), candidates.end (),
            [] (const SerialPortInfo &a, const SerialPortInfo &b) { return a.port == b.port; }),
        candidates.end ());
    return candidates;
}

bool wait_for_openbci_welcome (
    Serial &serial, std::chrono::milliseconds timeout, std::string &response)
{
    response.clear ();
    const auto deadline = std::chrono::steady_clock::now () + timeout;
    int count = 0;
    while (std::chrono::steady_clock::now () < deadline)
    {
        unsigned char byte;
        int result = serial.read_from_serial_port (&byte, 1);
        if (result < 0 || std::chrono::steady_clock::now () >= deadline)
        {
            return false;
        }
        if (result == 1)
        {
            if (response.size () < 4096)
            {
                response += (char)byte;
            }
            count = (byte == '$') ? count + 1 : 0;
            if (count == 3)
            {
                return true;
            }
        }
        else
        {
            // Some drivers return empty reads immediately; do not spin or count them as time.
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
    }
    return false;
}
