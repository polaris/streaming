// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#include "Utils.h"
#include <cmath>
#include <iostream>

uint64_t timespec_us(const struct timespec *ts) {
    return ts->tv_sec * 1000000LLU + ts->tv_nsec / 1000LLU;
}

uint64_t read_clock(clockid_t clock) {
    struct timespec realtime;
    clock_gettime(clock, &realtime);
    return timespec_us(&realtime);
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    double u = ts.tv_sec % 86400;
    u += ts.tv_nsec / 1000000000.0;
    return u;
}
