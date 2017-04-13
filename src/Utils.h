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

uint64_t timespec_us(const struct timespec *ts);

int64_t frames_us(unsigned int sample_rate, snd_pcm_sframes_t frames);

uint64_t read_clock(clockid_t clock);

double get_time();

#endif  // __UTILS_H
