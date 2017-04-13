#include "Receiver.h"
#include "CircularBuffer.h"
#include "Resampler.h"
#include "Packet.h"
#include "Utils.h"

#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

Receiver::Receiver(const std::string& mcastgroup, unsigned short port, unsigned int sampleRate,
    unsigned int periodTime, unsigned int periodSize, unsigned int channels, unsigned int latency,
    CircularBuffer& buffer, ReaderWriterQueue<timeinfo>& queue, std::atomic<bool>& streaming)
: mcastgroup_(mcastgroup)
, port_(port)
, periodTime_(periodTime)
, periodSize_(periodSize)
, channels_(channels)
, latency_(latency)
, socket_(0)
, addr_()
, buffer_(buffer)
, streaming_(streaming)
, thread_(nullptr)
, sampleCount_(0)
, ratio_(1.0)
, tA1(0)
, kA1(-periodSize_)
, timeInfoQueue_(queue)
, dll_(periodTime * 0.000001)
, est_(periodSize_, sampleRate)
, err_(0) {
}

Receiver::~Receiver() {
    stop();
}

void Receiver::start() {
    stop();

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    assert(socket_ != -1);

    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port_);

    int result = 0, enable = 1;
    result = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (int*)&enable, sizeof(enable));
    assert(result == 0);

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(mcastgroup_.c_str());
    mreq.imr_interface.s_addr = htonl (INADDR_ANY);
    result = setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    assert(result == 0);

    result = bind(socket_, (struct sockaddr*)&addr_, sizeof(addr_));
    assert(result == 0);

    thread_.reset(new std::thread([this] () {
        receive();
    }));
}

void Receiver::stop() {
    if (thread_ && thread_->joinable()) {
        if (socket_ != 0) {
            shutdown(socket_, SHUT_RDWR);
            socket_ = 0;
        }
        thread_->join();
    }
    thread_.reset();
}

void Receiver::receive() {
    Packet packet(channels_ * periodSize_ * sizeof(int16_t));

    Resampler resampler(periodSize_, channels_);

    dll_.reset(get_time());
    tA1 = dll_.t1();

    unsigned int counter = 0;

    while (1) {
        const auto n = recv(socket_, packet.packet_, packet.packetSize_, 0);
        if (n > 0) {
            dll_.update(get_time());

            if (streaming_) {
                const double tN = dll_.t0();

                timeinfo ti;
                if (timeInfoQueue_.try_dequeue(ti)) {
                    tA0 = tA1;
                    kA0 = kA1;
                    tA1 = ti.t1_;
                    kA1 += ti.k1_;
                }

                double tD = tN - tA0;
                if (tD > 0) {
                    unsigned int kN = sampleCount_ + periodSize_;
                    double dA = (kA1 - kA0) * tD / (tA1 - tA0);
                    double dN = kN - kA0;
                    err_ = dN - dA - (latency_ * periodSize_);
                    ratio_ = est_.estimateRatio(err_);
                    if (ratio_ > 1.05) {
                        ratio_ = 1.05;
                    }
                    if (ratio_ < 0.95) {
                        ratio_ = 0.95;
                    }
                    resampler.setRatio(ratio_);
                }

                resampler.convert(reinterpret_cast<int16_t*>(packet.data_));
                sampleCount_ += resampler.getFramesGenerated();

                const auto sample = packet.getTimestamp();
                buffer_.write(sample, resampler.getOutput(), resampler.getFramesGenerated());

                if (counter % 1000 == 0) {
                    std::cout << "Resampling ratio: " << ratio_ << ", delay error: " << err_ << ", " << buffer_.readWriteDiff() << "\n";
                }
                counter += 1;
            }

        } else if (n == 0) {
            break;
        } else {
            std::cerr << "Error: " << strerror(static_cast<int>(n)) << "\n";
            break;
        }
    }
}
