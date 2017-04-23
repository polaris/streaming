// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __CIRCULARBUFFER_H
#define __CIRCULARBUFFER_H

#include <cstddef>
#include <cstring>

/** A circular buffer specialized for adaptive resampling.
 */
class CircularBuffer {
public:
    /** Constructor
     *
     *  \param periodSize the size of a period in frames.
     *  \param channels the number of channels in each frame.
     *  \param latency the target latency.
     */
    CircularBuffer(unsigned int periodSize, unsigned int channels, unsigned int latency)
    : periodSize_(periodSize)
    , channels_(channels)
    , latency_(latency)
    , capacity_(periodSize_ * channels_ * 2 * latency_)
    , data_(new int16_t [capacity_])
    , lastRead_(0)
    , lastWrite_(0) {
        memset(data_, 0, capacity_ * sizeof(int16_t));
    }

    CircularBuffer(const CircularBuffer&) = delete;
    CircularBuffer& operator =(const CircularBuffer&) = delete;

    /** Destructor
     */
    ~CircularBuffer() {
        delete [] data_;
    }

    /** Index operator (const)
     *
     *  \param sample the index of the sample.
     */
    int16_t operator [] (int sample) const {
        const auto pos = (sample % (periodSize_ * 2 * latency_)) * channels_;
        return data_[pos];
    }

    /** Index operator (non-const)
     *
     *  \param sample the index of the sample.
     */
    int16_t& operator [] (int sample) {
        const auto pos = (sample % (periodSize_ * 2 * latency_)) * channels_;
        return data_[pos];
    }

    /** Writes frames to the buffer.
     *
     *  \param sample the index of the first sample to write.
     *  \param data a pointer to data that should be written to the buffer.
     *  \param length the number frames in the input.
     */
    void write(int32_t sample, int16_t* data, uint32_t length) {
        const auto pos = (sample % (periodSize_ * 2 * latency_)) * channels_;
        lastWrite_ = pos;
        auto rest = static_cast<int>(pos + (length * channels_)) - static_cast<int>(capacity_);
        if (rest < 0) {
            rest = 0;
        }
        const auto n = ((length * channels_) - rest);
        memcpy(data_ + pos, data, n * sizeof(int16_t));
        if (rest > 0) {
            memcpy(data_, data + n, rest * sizeof(int16_t));
        }
    }

    /** Reads frames from the buffer.
     *
     *  \param sample the index of the first sample to read.
     *  \param data a pointer to the output memory.
     *  \param length the number of frames to read from the buffer.
     */
    void read(int32_t sample, int16_t* data, uint32_t length) {
        const auto pos = (sample % (periodSize_ * 2 * latency_)) * channels_;
        lastRead_ = pos;
        auto rest = static_cast<int>(pos + (length * channels_)) - static_cast<int>(capacity_);
        if (rest < 0) {
            rest = 0;
        }
        const auto n = ((length * channels_) - rest);
        memcpy(data, data_ + pos, n * sizeof(int16_t));
        if (rest > 0) {
            memcpy(data + n, data_, rest * sizeof(int16_t));
        }
    }

    /** Returns the difference between the last read and the last write in frames.
     */
    int32_t readWriteDiff() const {
        const auto diff = lastWrite_ > lastRead_ ? lastWrite_ - lastRead_ : lastRead_ - lastWrite_;
        return diff / channels_;
    }

private:
    const unsigned int periodSize_;     /**< The size of one period in frames.      */
    const unsigned int channels_;       /**< The number of channels in each frame.  */
    const unsigned int latency_;        /**< The target latency in periods.         */
    const unsigned int capacity_;       /**< The capacity of the buffer.            */
    int16_t * const data_;              /**< A pointer to the buffer data.          */
    int32_t lastRead_;                  /**< The index of the last read.            */
    int32_t lastWrite_;                 /**< The index of the last write.           */
};

#endif  // __CIRCULARBUFFER_H
