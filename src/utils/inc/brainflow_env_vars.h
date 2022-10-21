#pragma once

#include <cstdlib>
#include <string.h>
#include <string>


inline int get_brainflow_batch_size (int default_size = 3)
{
    int size = default_size;
    if (const char *env_p = std::getenv ("BRAINFLOW_BATCH_SIZE"))
    {
        std::string str_env = env_p;
        try
        {
            int parsed_size = std::stoi (str_env);
            if ((parsed_size > 1) && (parsed_size < 100))
            {
                size = parsed_size;
            }
        }
        catch (...)
        {
        }
    }
    return size;
}
