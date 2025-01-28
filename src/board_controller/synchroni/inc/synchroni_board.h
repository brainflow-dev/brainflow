#include <condition_variable>
#include <mutex>
#include <thread>


#include "board.h"
#include "board_controller.h"
#include "runtime_dll_loader.h"

class SynchroniBoard : public Board
{
protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    std::shared_ptr<DLLLoader> dll_loader;
    static std::shared_ptr<DLLLoader> g_dll_loader;

    virtual int call_init ();
    virtual int call_open ();
    virtual int call_close ();
    virtual int call_start ();
    virtual int call_stop ();
    virtual int call_release ();

    virtual void read_thread ();
    virtual std::string get_lib_name ();


public:
    SynchroniBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~SynchroniBoard ();

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
