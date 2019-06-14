#ifndef OPENBCI_SERIAL_BOARD
#define OPENBCI_SERIAL_BOARD

#include <thread>

#include "board.h"
#include "board_controller.h"
#include "serial.h"

#define MAX_CAPTURE_SAMPLES (86400 * 250) // should be enough for one day of capturing

class OpenBCISerialBoard : public Board
{

protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;

    Serial serial;

    DataBuffer *db;
    int num_channels;

    virtual int open_port ();
    virtual int send_to_board (char *message);
    virtual int status_check ();
    virtual int set_port_settings ();
    virtual void read_thread () = 0;

public:
    OpenBCISerialBoard (int num_channels, const char *port_name);
    virtual ~OpenBCISerialBoard ();

    virtual int prepare_session ();
    virtual int start_stream (int buffer_size);
    virtual int stop_stream ();
    virtual int release_session ();
};

#endif
