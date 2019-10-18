#ifndef BRAINFLOW_EXCEPTION
#define BRAINFLOW_EXCEPTION

#include <exception>
#include <string>

class BrainFlowException : public std::exception
{

public:
    int exit_code;

    explicit BrainFlowException (const char *msg, int exit_code) : std::exception ()
    {
        msg_ = std::string (msg) + std::to_string (exit_code);
        this->exit_code = exit_code;
    }

    virtual ~BrainFlowException () throw ()
    {
    }

    virtual const char *what () const throw ()
    {
        return msg_.c_str ();
    }

protected:
    std::string msg_;
};

#endif
