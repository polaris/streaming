// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __RESAMPLER_H
#define __RESAMPLER_H

#include <cassert>
#include <samplerate.h>

/** A simple wrapper around libsamplerate.
 */
class Resampler {
public:
    /** Constructor
     *
     *  \param periodSize the size of a period in frames.
     *  \param channels the number of channels per frame.
     */
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

    /** Destructor.
     */
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

    /** Sets the resampling ratio.
     *
     *  \param ratio the new resampling ratio.
     */
    void setRatio(double ratio) {
        ratio_ = ratio;
    }

    /** Converts the sample rate of a period of audio data.
     */
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

    /** Returns the number of generated frames.
     */
    unsigned int getFramesGenerated() const {
        return framesGenerated_;
    }

    /** Returns a pointer to the output data.
     */
    short* getOutput() const {
        return outputBufferShort;
    }

private:
    const unsigned int periodSize_;     /**< The period size in frames.         */
    const unsigned int channels_;       /**< The number of channels in a frame. */
    SRC_STATE* state_;                  /**< The state of libsamplerate.        */
    float* inputBufferFloat;            /**< A pointer to an array of float for the input data.     */
    float* outputBufferFloat;           /**< A pointer to an array of float for the output data.    */
    short* outputBufferShort;           /**< A pointer to an array of short for the output data.    */
    double ratio_;                      /**< The current resampling ratio.      */
    unsigned int framesGenerated_;      /**< The number of generated frames.    */
};

#endif  // __RESAMPLER_H
