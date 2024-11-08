#include <condition_variable>
#include <mutex>
#include <thread>


#include "dyn_lib_board.h"
#include "board_controller.h"


class SynchroniBoard : public DynLibBoard
{
public:
    SynchroniBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~SynchroniBoard ();

    int config_board (std::string config, std::string &response);

protected:
    virtual int call_init ();
    virtual int call_open ();
    virtual int call_close ();
    virtual int call_start ();
    virtual int call_stop ();
    virtual int call_release ();

    virtual void read_thread ();
    virtual std::string get_lib_name ();
};
