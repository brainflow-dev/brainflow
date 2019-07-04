#ifndef BRAINFLOW_EXCEPTION
#define BRAINFLOW_EXCEPTION

#include <exception>
#include <string>

class BrainFlowException : public std::exception
{
    int exit_code;
    std::string message;

public:
    BrainFlowException (const char *msg, int exit_code_)
        : std::exception (), exit_code (exit_code_), message (msg)
    {
    }

    const char *what () const throw ()
    {
        return (message + std::string (":") + std::to_string (exit_code)).c_str ();
    }

    int get_exit_code () const
    {
        return exit_code;
    }
};

#endif
