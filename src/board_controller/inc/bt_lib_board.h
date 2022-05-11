#pragma once

#include <string>

#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"


class BTLibBoard : public Board
{

protected:
    bool initialized;
    DLLLoader *dll_loader;

    int bluetooth_open_device ();
    int bluetooth_get_data (char *data, int len);
    int bluetooth_write_data (const char *data, int len);
    int bluetooth_close_device ();
    int find_bt_addr (const char *name_selector);

    virtual std::string get_name_selector () = 0;

public:
    BTLibBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~BTLibBoard ();

    virtual int prepare_session ();
    virtual int release_session ();
    virtual int config_board (std::string config, std::string &response);
};
