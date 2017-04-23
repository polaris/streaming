// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __DELAYLOCKEDLOOP_H
#define __DELAYLOCKEDLOOP_H

#include "Utils.h"

#include <cstdint>
#include <ctime>
#include <cmath>
#include <unistd.h>

/** An implementation of a delay-locked loop as described by Fons Adriaensen.
 *  in his paper "Using a DLL to Filter Time" from 2005.
 *
 *  http://kokkinizita.linuxaudio.org/papers/usingdll.pdf
 */
class DelayLockedLoop {
public:
    /** Constructor
     *
     *  \param periodTimeUs the expected period time in microseconds.
     */
    DelayLockedLoop(double periodTimeUs)
    : tper_(periodTimeUs)
    , b_(0), c_(0), t0_(0), t1_(0), e2_(0) {
        const double sqrt2 = 1.414213562373095;
        const double pi = 3.141592653589793;
        const double omega = 2.0 * pi * 0.1 * tper_;
        b_ = sqrt2 * omega;
        c_ = omega * omega;
    }

    /** Resets the state to a new start time.
     *
     *  \param t the current system time.
     */
    void reset(double t) {
        e2_ = tper_;
        t0_ = t;
        t1_ = t0_ + e2_;
    }

    /** Updates the estimation.
     *
     *  \param t the current system time.
     */
    void update(double t) {
        const double e = t - t1_;
        t0_ = t1_;
        t1_ = t1_ + b_ * e + e2_;
        e2_ += c_ * e;
    }

    /** Returns the estimated current time in seconds.
     */
    inline double t0() const { return t0_; }

    /** Returns the estimated time of the next cycle in seconds.
     */
    inline double t1() const { return t1_; }

    /** Returns the distance between the estimated current time and estimated time of the next cycle.
     */
    inline double periodTime() const { return t1_ - t0_; }

private:
    const double tper_;     /**< The expected period time in microsecond.           */
    double b_;              /**< Coefficient b                                      */
    double c_;              /**< Coefficient c                                      */
    double t0_;             /**< The estimated current time in seconds.             */
    double t1_;             /**< The estimated time of the next cycle in seconds.   */
    double e2_;             /**< The integrated acceleration.                       */
};

#endif  // __DELAYLOCKEDLOOP_H
