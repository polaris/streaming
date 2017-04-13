#ifndef __PLAYER_H
#define __PLAYER_H

#include "Utils.h"

#include <readerwriterqueue.h>
#include <thread>
#include <memory>
#include <atomic>
#include <alsa/asoundlib.h>
#include <cstddef>

class CircularBuffer;
class Filter;

class Player {
public:
    Player(const std::string& deviceName, unsigned int sampleRate, unsigned int periodTime, unsigned int channels, unsigned int latency, CircularBuffer& buffer, moodycamel::ReaderWriterQueue<timeinfo>& q, std::atomic<bool>& streaming);

    Player(const Player&) = delete;
    Player& operator =(const Player&) = delete;

    ~Player();

    void start();

    void stop();

private:
    void setUpAlsa();
    void setHardwareParameters();
    void setSoftwareParameters();
    void tearDownAlsa();
    int playback();
    int recover(int err);

    const std::string deviceName_;
    const unsigned int sampleRate_;
    const unsigned int periodTime_;
    const unsigned int periodSize_;
    const unsigned int channels_;
    const unsigned int latency_;

    CircularBuffer& buffer_;
    std::atomic<bool>& streaming_;
    moodycamel::ReaderWriterQueue<timeinfo>& timeInfoQueue_;
    snd_pcm_t* pcm_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_;
};

#endif  // __PLAYER_H
