#include "ant_neuro_edx.h"

#include "get_dll_dir.h"

AntNeuroEdxBoard::AntNeuroEdxBoard (int board_id, struct BrainFlowInputParams params)
    : DynLibBoard (board_id, params)
{
}

AntNeuroEdxBoard::~AntNeuroEdxBoard ()
{
}

std::string AntNeuroEdxBoard::get_lib_name ()
{
    std::string ant_edx_lib_path = "";
    std::string ant_edx_lib_name = "";
    char ant_edx_lib_dir[1024];
    bool res = get_dll_path (ant_edx_lib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        ant_edx_lib_name = "AntNeuroEdxLib32.dll";
    }
    else
    {
        ant_edx_lib_name = "AntNeuroEdxLib.dll";
    }
#endif
#ifdef __linux__
    ant_edx_lib_name = "libAntNeuroEdxLib.so";
#endif
#ifdef __APPLE__
    ant_edx_lib_name = "libAntNeuroEdxLib.dylib";
#endif

    if (res)
    {
        ant_edx_lib_path = std::string (ant_edx_lib_dir) + ant_edx_lib_name;
    }
    else
    {
        ant_edx_lib_path = ant_edx_lib_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", ant_edx_lib_path.c_str ());
    return ant_edx_lib_path;
}
