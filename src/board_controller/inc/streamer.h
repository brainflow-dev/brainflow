#pragma once

#include <string>

class Streamer
{
public:
    Streamer (int len, std::string type, std::string dest, std::string mods)
    {
        this->len = len;
        streamer_type = type;
        streamer_dest = dest;
        streamer_mods = mods;
    }

    virtual ~Streamer ()
    {
    }

    virtual int init_streamer () = 0;
    virtual void stream_data (double *data) = 0;

    virtual bool check_equals (std::string type, std::string dest, std::string mods)
    {
        return ((streamer_type == type) && (streamer_dest == dest) && (streamer_mods == mods));
    }

protected:
    std::string streamer_type;
    std::string streamer_dest;
    std::string streamer_mods;
    int len;
};
