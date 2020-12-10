#pragma once

#include <cstdlib>
#include <string>
#include <thread>


inline int get_num_threads ()
{
    int num_system_threads = std::thread::hardware_concurrency ();
    char *omp_num_threads = std::getenv ("OMP_NUM_THREADS");
    if (omp_num_threads == NULL)
    {
        return num_system_threads;
    }
    int env_threads = 1;
    std::string omp_env (omp_num_threads);
    try
    {
        env_threads = std::stoi (omp_env);
    }
    catch (...)
    {
        return num_system_threads;
    }
    if ((env_threads < 1) || (env_threads > num_system_threads))
    {
        return num_system_threads;
    }
    return env_threads;
}
