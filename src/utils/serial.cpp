#include "serial.h"
#include "libftdi_serial.h"
#include "os_serial.h"


Serial *Serial::create (const char *port_name, Board *board)
{
#ifdef USE_LIBFTDI
    {
        LibFTDISerial ftdi_probe (port_name);
        if (ftdi_probe.open_serial_port () != SerialExitCodes::PORT_NAME_ERROR)
        {
            return new LibFTDISerial (port_name, board);
        }
    }
#endif

    return new OSSerial (port_name);
}
