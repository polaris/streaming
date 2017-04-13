#ifndef __RECEIVER_H
#define __RECEIVER_H

#include "DelayLockedLoop.h"
#include "ResampleRatioEstimator.h"

#include <readerwriterqueue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <memory>

using namespace moodycamel;

class CircularBuffer;
class Filter;

class Receiver {
public:
    Receiver(const std::string& address, unsigned short port, unsigned int sampleRate,
        unsigned int periodTime, unsigned int periodSize, unsigned int channels, unsigned int latency,
        CircularBuffer& buffer, ReaderWriterQueue<timeinfo>& queue, std::atomic<bool>& streaming);

    ~Receiver();

    void start();

    void stop();

private:
    void receive();

    const std::string mcastgroup_;
    const unsigned short port_;
    const unsigned int periodTime_;
    const unsigned int periodSize_;
    const unsigned int channels_;
    const unsigned int latency_;

    int socket_;
    struct sockaddr_in addr_;
    CircularBuffer& buffer_;
    std::atomic<bool>& streaming_;
    std::unique_ptr<std::thread> thread_;

    unsigned int sampleCount_;
 
    double ratio_;
    double tA0, tA1;
    unsigned int kA0, kA1;
    ReaderWriterQueue<timeinfo>& timeInfoQueue_;
    DelayLockedLoop dll_;
    ResampleRatioEstimator est_;
    double err_;
};

#endif  // __RECEIVER_H
