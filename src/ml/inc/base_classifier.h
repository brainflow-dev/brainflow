#pragma once


class BaseClassifier
{
public:
    BaseClassifier ()
    {
    }

    virtual ~BaseClassifier ()
    {
    }

    virtual int prepare () = 0;
    virtual int predict (double *data, int data_len, double *output) = 0;
    virtual int release () = 0;
};