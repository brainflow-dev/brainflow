#pragma once

#include <string>

#include "muse_bglib_helper.h"


class Muse2BGLibHelper : public MuseBGLibHelper
{
public:
    Muse2BGLibHelper () : MuseBGLibHelper ()
    {
    }

    std::string get_preset ()
    {
        return "p21";
    }

    int get_buffer_size ()
    {
        return 10;
    }
};
