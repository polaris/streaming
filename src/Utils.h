// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __UTILS_H
#define __UTILS_H

#include <alsa/asoundlib.h>

#include <cstdint>
#include <ctime>

struct timeinfo {
	timeinfo()
	: t1_(0)
	, k1_(0) {
	}
	timeinfo(double t, unsigned int k)
	: t1_(t)
	, k1_(k) {
	}
	timeinfo(const timeinfo& ti) 
	: t1_(ti.t1_)
	, k1_(ti.k1_) {
	}

    double t1_;
    unsigned int k1_;
};

/** Returns the current value of the specified clock in microseconds.
 */
uint64_t read_clock(clockid_t clock);

/** Returns the current real-time in microseconds.
 */
double get_time();

#endif  // __UTILS_H
