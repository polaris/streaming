#ifndef __CIRCULARBUFFER_H
#define __CIRCULARBUFFER_H

#include <cstddef>
#include <cstring>

class CircularBuffer {
public:
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

    ~CircularBuffer() {
        delete [] data_;
    }

    int16_t operator [] (int sample) const {
        const auto pos = (sample % (periodSize_ * 2 * latency_)) * channels_;
        return data_[pos];
    }

    int16_t& operator [] (int sample) {
        const auto pos = (sample % (periodSize_ * 2 * latency_)) * channels_;
        return data_[pos];
    }

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

    int32_t readWriteDiff() const {
        const auto diff = lastWrite_ > lastRead_ ? lastWrite_ - lastRead_ : lastRead_ - lastWrite_;
        return diff / channels_;
    }

private:
    const unsigned int periodSize_;
    const unsigned int channels_;
    const unsigned int latency_;
    const unsigned int capacity_;
    int16_t * const data_;
    int32_t lastRead_, lastWrite_;
};

#endif  // __CIRCULARBUFFER_H
