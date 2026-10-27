#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "brainflow_constants.h"
#include "openbci_serial_helpers.h"
#include "serial_port_info.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

static void require (bool value, const char *message)
{
    if (!value)
    {
        throw std::runtime_error (message);
    }
}

static SerialPortInfo port (
    const std::string &name, const std::string &serial, int vid = 0x0403, int pid = 0x6015)
{
    SerialPortInfo result;
    result.port = name;
    result.serial_number = serial;
    result.vid = vid;
    result.pid = pid;
    return result;
}

static void test_candidates ()
{
    const int cyton = (int)BoardIds::CYTON_BOARD;
    auto a = port ("COM10", "A");
    auto b = port ("COM2", "B");
    auto unrelated = port ("COM3", "A", 0x0403, 0x6001);
    require (get_openbci_serial_candidates (cyton, {}, "").empty (), "empty enumeration");
    require (get_openbci_serial_candidates (cyton, {unrelated}, "").empty (), "wrong PID");
    require (get_openbci_serial_candidates (cyton, {port ("X", "A", -1, -1)}, "").empty (),
        "missing USB IDs");
    require (get_openbci_serial_candidates (cyton, {port ("X", "A", 1)}, "").empty (), "wrong VID");
    require (
        get_openbci_serial_candidates (cyton, {port ("", "A")}, "").empty (), "missing port name");
    require (get_openbci_serial_candidates (cyton, {a, unrelated}, "").size () == 1,
        "unique candidate among unrelated adapters");
    auto ambiguous = get_openbci_serial_candidates (cyton, {b, a}, "");
    require (ambiguous.size () == 2, "two identical devices must remain ambiguous");
    auto reversed = get_openbci_serial_candidates (cyton, {a, b}, "");
    require (ambiguous[0].port == reversed[0].port && ambiguous[1].port == reversed[1].port,
        "enumeration ordering must not change the candidates");
    auto selected = get_openbci_serial_candidates (cyton, {a, b, unrelated}, "B");
    require (selected.size () == 1 && selected[0].port == "COM2", "USB serial selection");
    require (get_openbci_serial_candidates (cyton, {a, b}, "missing").empty (),
        "serial mismatch must not fall back");
    require (get_openbci_serial_candidates (cyton, {port ("COM1", "")}, "A").empty (),
        "unknown serial must not match a serial filter");
    require (get_openbci_serial_candidates (cyton, {a, port ("COM2", "A")}, "A").size () == 2,
        "duplicate USB serials must remain ambiguous");
    require (get_openbci_serial_candidates (cyton, {a, a}, "").size () == 1,
        "duplicate OS entries must not create ambiguity");
    require (
        get_openbci_serial_candidates ((int)BoardIds::CYTON_DAISY_BOARD, {a}, "A").size () == 1,
        "Daisy uses the same dongle");
    require (get_openbci_serial_candidates ((int)BoardIds::GANGLION_BOARD, {a}, "").empty (),
        "no generic USB fallback for other boards");
}

static void test_windows_metadata ()
{
    SerialPortInfo value;
    require (serial_port_info::parse_windows_usb_instance (
                 "USB\\VID_0403&PID_6015\\DM12345", true, value) &&
            value.vid == 0x0403 && value.pid == 0x6015 && value.serial_number == "DM12345",
        "physical USB instance");
    require (serial_port_info::parse_windows_usb_instance (
                 "usb\\vid_0403&pid_6015\\5&LOCATION&0&1", false, value) &&
            value.serial_number.empty (),
        "do not expose Windows location IDs as USB serials");
    for (const char *invalid : {"", "USB\\VID_", "USB\\VID_0403&PID_6015\\",
             "USB\\VID_0403&PID_6015&MI_00\\6&CHILD", "FTDIBUS\\VID_0403+PID_6015+A\\0000",
             "USB\\VID_zzzz&PID_6015\\A", "USB\\VID_0403&PID_601x\\A"})
    {
        require (!serial_port_info::parse_windows_usb_instance (invalid, true, value),
            "malformed IDs and non-physical USB children must be rejected");
    }
    require (serial_port_info::parse_usb_id ("aB09") == 0xab09, "mixed case hex");
    require (serial_port_info::parse_usb_id ("0403junk") == -1, "reject trailing data");
    require (serial_port_info::parse_usb_id ("+403") == -1, "reject signed hex");
}

class FakeSerial : public Serial
{
public:
    int reads = 0;
    int empty_reads = 0;
    bool error = false;
    bool noise = false;
    int read_delay_ms = 0;
    std::string data;

    int read_from_serial_port (void *bytes, int) override
    {
        reads++;
        if (read_delay_ms)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (read_delay_ms));
        }
        if (error)
        {
            return -1;
        }
        if (reads <= empty_reads)
        {
            return 0;
        }
        if (noise)
        {
            *(unsigned char *)bytes = 'x';
            return 1;
        }
        if (data.empty ())
        {
            return 0;
        }
        *(unsigned char *)bytes = (unsigned char)data[0];
        data.erase (0, 1);
        return 1;
    }
    int open_serial_port () override
    {
        throw std::runtime_error ("unexpected open");
    }
    bool is_port_open () override
    {
        return true;
    }
    int set_serial_port_settings (int, bool) override
    {
        return 0;
    }
    int set_custom_baudrate (int) override
    {
        return 0;
    }
    int set_custom_latency (int) override
    {
        return 0;
    }
    int flush_buffer () override
    {
        throw std::runtime_error ("flushed after reset");
    }
    int send_to_serial_port (const void *, int) override
    {
        throw std::runtime_error ("unexpected write");
    }
    int close_serial_port () override
    {
        return 0;
    }
    const char *get_port_name () override
    {
        return "fake";
    }
};

static void test_welcome ()
{
    std::string response;
    FakeSerial delayed;
    delayed.empty_reads = 10;
    delayed.data = "OpenBCI$$x$$$";
    require (wait_for_openbci_welcome (delayed, std::chrono::seconds (2), response),
        "welcome after more than six empty reads");
    require (response == "OpenBCI$$x$$$", "fragmented terminator and reset of dollar count");

    FakeSerial silent;
    auto start = std::chrono::steady_clock::now ();
    require (!wait_for_openbci_welcome (silent, std::chrono::milliseconds (40), response),
        "silent device must time out");
    auto elapsed = std::chrono::steady_clock::now () - start;
    require (elapsed >= std::chrono::milliseconds (40) && elapsed < std::chrono::seconds (2),
        "elapsed-time deadline");
    require (silent.reads < 100, "empty reads must back off");

    FakeSerial noisy;
    noisy.noise = true;
    require (!wait_for_openbci_welcome (noisy, std::chrono::milliseconds (40), response),
        "continuous noise must not extend the deadline");
    require (response.size () <= 4096, "bounded diagnostic buffer");

    FakeSerial long_welcome;
    long_welcome.data = std::string (5000, 'x') + "$$$";
    require (wait_for_openbci_welcome (long_welcome, std::chrono::seconds (2), response),
        "welcome is not limited by old 500-read cap or diagnostic buffer");

    FakeSerial disconnected;
    disconnected.error = true;
    require (!wait_for_openbci_welcome (disconnected, std::chrono::seconds (2), response) &&
            disconnected.reads == 1,
        "read error must stop immediately");

    FakeSerial late;
    late.read_delay_ms = 20;
    late.data = "$$$";
    require (!wait_for_openbci_welcome (late, std::chrono::milliseconds (10), response),
        "read completing after deadline must not succeed");
}

#if defined(__linux__) && !defined(__ANDROID__)
static void test_linux_metadata ()
{
    char temp[] = "/tmp/brainflow-serial-XXXXXX";
    char *created = mkdtemp (temp);
    require (created != NULL, "create fixture directory");
    const std::string root = created;
    const std::vector<std::string> directories = {"/sys", "/dev", "/usb", "/usb/interface",
        "/usb/interface/tty", "/sys/ttyUSB0", "/sys/ttyUSB1", "/sys/ttyS0", "/sys/ttyUSB2"};
    for (const auto &dir : directories)
    {
        require (mkdir ((root + dir).c_str (), 0700) == 0, "create fixture hierarchy");
    }
    std::ofstream (root + "/usb/idVendor") << "0403\n";
    std::ofstream (root + "/usb/idProduct") << "6015\n";
    std::ofstream (root + "/usb/serial") << "DONGLE_A\n";
    for (const auto &name : {"ttyUSB0", "ttyUSB1"})
    {
        require (symlink ((root + "/usb/interface/tty").c_str (),
                     (root + "/sys/" + name + "/device").c_str ()) == 0,
            "create sysfs link");
    }
    // A FIFO would block if enumeration opened the serial node; metadata lookup must not do so.
    require (mkfifo ((root + "/dev/ttyUSB0").c_str (), 0600) == 0, "create unopened device node");
    std::ofstream (root + "/dev/ttyS0");
    std::ofstream (root + "/dev/ttyUSB2");
    require (symlink ((root + "/missing").c_str (), (root + "/sys/ttyUSB2/device").c_str ()) == 0,
        "create disconnected device link");
    std::vector<SerialPortInfo> ports;
    require (serial_port_info::enumerate_linux (ports, root + "/sys", root + "/dev") == 0 &&
            ports.size () == 1 && ports[0].port == root + "/dev/ttyUSB0" &&
            ports[0].vid == 0x0403 && ports[0].pid == 0x6015 &&
            ports[0].serial_number == "DONGLE_A",
        "sysfs USB ancestor, missing device nodes, non-USB and hot-unplug handling");
    unlink ((root + "/usb/serial").c_str ());
    require (serial_port_info::enumerate_linux (ports, root + "/sys", root + "/dev") == 0 &&
            ports.size () == 1 && ports[0].serial_number.empty (),
        "optional USB serial");
    std::ofstream (root + "/usb/idVendor") << "bad!\n";
    require (serial_port_info::enumerate_linux (ports, root + "/sys", root + "/dev") == 0 &&
            ports.empty (),
        "malformed sysfs IDs");
    require (serial_port_info::enumerate_linux (ports, root + "/missing", root + "/dev") ==
            SerialExitCodes::ENUMERATION_ERROR,
        "enumeration failure differs from zero ports");
    for (const auto &file : {"/usb/idVendor", "/usb/idProduct", "/dev/ttyUSB0", "/dev/ttyS0",
             "/dev/ttyUSB2", "/sys/ttyUSB0/device", "/sys/ttyUSB1/device", "/sys/ttyUSB2/device"})
    {
        unlink ((root + file).c_str ());
    }
    for (auto it = directories.rbegin (); it != directories.rend (); ++it)
    {
        rmdir ((root + *it).c_str ());
    }
    rmdir (root.c_str ());
}
#endif

int main ()
{
    try
    {
        test_candidates ();
        test_windows_metadata ();
        test_welcome ();
#if defined(__linux__) && !defined(__ANDROID__)
        test_linux_metadata ();
#endif
        std::vector<SerialPortInfo> ports;
        require (
            Serial::get_port_info (ports) == SerialExitCodes::OK, "native enumeration smoke test");
        std::cout << "Serial discovery and welcome tests passed (" << ports.size ()
                  << " native USB serial ports)." << std::endl;
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what () << std::endl;
        return 1;
    }
}
