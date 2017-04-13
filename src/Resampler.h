#ifndef __RESAMPLER_H
#define __RESAMPLER_H

#include <cassert>
#include <samplerate.h>

class Resampler {
public:
    Resampler(unsigned int periodSize, unsigned int channels)
    : periodSize_(periodSize)
    , channels_(channels)
    , state_(nullptr)
    , inputBufferFloat(nullptr)
    , outputBufferFloat(nullptr)
    , outputBufferShort(nullptr)
    , ratio_(1.0)
    , framesGenerated_(0) {
        int err = 0;
        state_ = src_new(SRC_SINC_MEDIUM_QUALITY, channels_, &err);
        assert(state_ != nullptr);

        inputBufferFloat = new float [periodSize_ * channels_];
        outputBufferFloat = new float [periodSize_ * channels_ * 2];
        outputBufferShort = new short [periodSize_ * channels_ * 2];
    }

    ~Resampler() {
        if (state_ != nullptr) {
            src_delete(state_);
        }
        if (inputBufferFloat) {
            delete [] inputBufferFloat;
        }
        if (outputBufferFloat) {
            delete [] outputBufferFloat;
        }
        if (outputBufferShort) {
            delete [] outputBufferShort;
        }
    }

    void setRatio(double ratio) {
        ratio_ = ratio;
    }

    void convert(int16_t* data) {
        src_short_to_float_array(data, inputBufferFloat, periodSize_ * channels_);
        SRC_DATA srcData;
        srcData.data_in = inputBufferFloat;
        srcData.data_out = outputBufferFloat;
        srcData.input_frames = periodSize_;
        srcData.output_frames = periodSize_ * 2;
        srcData.end_of_input = 0;
        srcData.src_ratio = ratio_;
        int result = src_process(state_, &srcData);
        assert(result == 0);
        src_float_to_short_array(outputBufferFloat, outputBufferShort, srcData.output_frames_gen * channels_);
        framesGenerated_ = srcData.output_frames_gen;
    }

    unsigned int getFramesGenerated() const {
        return framesGenerated_;
    }

    short* getOutput() const {
        return outputBufferShort;
    }

private:
    const unsigned int periodSize_;
    const unsigned int channels_;
    SRC_STATE* state_;
    float* inputBufferFloat;
    float* outputBufferFloat;
    short* outputBufferShort;
    double ratio_;
    unsigned int framesGenerated_;
};

#endif  // __RESAMPLER_H
