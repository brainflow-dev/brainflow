#include <sys/time.h>
#include <stdlib.h>
#include "TimeStamp.h"

double get_timestamp ()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (double) (tv.tv_sec) + (double) (tv.tv_usec) / 1000;
}