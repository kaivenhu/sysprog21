#ifndef UTIME_H_
#define UTIME_H_
#include <time.h>

struct timespec timespec_diff(const struct timespec start,
                              const struct timespec stop);

unsigned long long timespec_to_ns(const struct timespec t);

#endif /* UTIME_H_ */
