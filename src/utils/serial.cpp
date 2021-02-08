#include "serial.h"
#include "libftdi_serial.h"
#include "os_serial.h"


Serial *Serial::create (const char *port_name, Board *board)
{
#ifdef USE_LIBFTDI
    if (LibFTDISerial::is_libftdi (port_name))
    {
        return new LibFTDISerial (port_name, board);
    }
#endif

    return new OSSerial (port_name);
}
