#pragma once

#include <string>

#include "muse_bglib_helper.h"


// for now its the same as for Muse2, will add MuseS specific data later
class MuseSBGLibHelper : public MuseBGLibHelper
{
public:
    MuseSBGLibHelper () : MuseBGLibHelper ()
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
