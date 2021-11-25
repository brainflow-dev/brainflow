#pragma once

#include <sstream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif


inline void append_to_search_path (std::string value)
{

#ifdef _WIN32
    std::string name = "PATH";
    std::string sep = ";";
#else
    std::string name = "LD_LIBRARY_PATH";
    std::string sep = ":";
#endif

    std::stringstream ss;
    ss << getenv (name.c_str ());
    ss << sep << value;
    ss << '\0';
    std::string new_val = ss.str ();
#ifdef _WIN32
    _putenv_s (name.c_str (), new_val.c_str ());
#else
    setenv (name.c_str (), new_val.c_str (), true);
#endif
}

inline std::string get_search_path ()
{
#ifdef _WIN32
    return getenv ("PATH");
#else
    return getenv ("LD_LIBRARY_PATH");
#endif
}