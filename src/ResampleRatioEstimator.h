#ifndef __RESAMPLERATIOESTIMATOR_H
#define __RESAMPLERATIOESTIMATOR_H

#include <cmath>

class ResampleRatioEstimator {
public:
    ResampleRatioEstimator(unsigned int periodSize, unsigned int sampleRate)
    : periodSize_(periodSize), sampleRate_(sampleRate)
    , w0_(0), w1_(0), w2_(0), z1_(0), z2_(0), z3_(0) {
        setBandwidth(0.1);
    }

    void setBandwidth(double bandWidth) {
        const double omega = 6.28 * bandWidth * periodSize_ / sampleRate_;
        w0_ = 1.0 - exp(-20.0 * omega);
        w1_ = omega * 1.5 / periodSize_;
        w2_ = omega / 1.5;
    }

    double estimateRatio(double err) {
        z1_ += w0_ * (w1_ * err - z1_);
        z2_ += w0_ * (z1_ - z2_);
        z3_ += w2_ * z2_;
        return 1.0 - (z2_ + z3_);
    }

private:
    const double periodSize_;
    const double sampleRate_;
    double w0_, w1_, w2_;
    double z1_, z2_, z3_;
};

#endif  // __RESAMPLERATIOESTIMATOR_H
