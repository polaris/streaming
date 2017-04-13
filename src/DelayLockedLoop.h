#ifndef __DELAYLOCKEDLOOP_H
#define __DELAYLOCKEDLOOP_H

#include "Utils.h"

#include <cstdint>
#include <ctime>
#include <cmath>
#include <unistd.h>

class DelayLockedLoop {
public:
    DelayLockedLoop(double periodTimeUs)
    : tper_(periodTimeUs)
    , b_(0)
    , c_(0)
    , t0_(0)
    , t1_(0)
    , e2_(0) {
        double sqrt2 = 1.414213562373095;
        double pi = 3.141592653589793;
        double omega = 2.0 * pi * 0.1 * tper_;
        b_ = sqrt2 * omega;
        c_ = omega * omega;
    }

    void reset(double t) {
        e2_ = tper_;
        t0_ = t;
        t1_ = t0_ + e2_;
    }

    void update(double t) {
        double e = t - t1_;
        t0_ = t1_;
        t1_ = t1_ + b_ * e + e2_;
        e2_ += c_ * e;
    }

    inline double t0() const {
        return t0_;
    }

    inline double t1() const {
        return t1_;
    }

    inline double periodTime() const {
        return t1_ - t0_;
    }

private:
    const double tper_;
    double b_;
    double c_;
    double t0_;
    double t1_;
    double e2_;
};

#endif  // __DELAYLOCKEDLOOP_H
