#include <unistd.h>

#include "utime.h"

struct timespec timespec_diff(const struct timespec start,
                              const struct timespec stop)
{
    struct timespec result;
    if ((stop.tv_nsec - start.tv_nsec) < 0) {
        result.tv_sec = stop.tv_sec - start.tv_sec - 1;
        result.tv_nsec = stop.tv_nsec - start.tv_nsec + 1000000000;
    } else {
        result.tv_sec = stop.tv_sec - start.tv_sec;
        result.tv_nsec = stop.tv_nsec - start.tv_nsec;
    }

    return result;
}

unsigned long long timespec_to_ns(const struct timespec t)
{
    return t.tv_sec * 1000000000.0 + t.tv_nsec;
}
