#ifndef __RECORDER_H
#define __RECORDER_H

#include <thread>
#include <memory>
#include <atomic>

#include <alsa/asoundlib.h>

#include <cstddef>

class Transmitter;
class PacketPool;

class Recorder {
public:
    enum class Mode {
        Capture,
        Click
    };

    Recorder(const std::string& deviceName, unsigned int sampleRate, unsigned int periodTime, unsigned int channels, Mode mode, Transmitter& transmitter, PacketPool& pool);

    Recorder(const Recorder&) = delete;
    Recorder& operator =(const Recorder&) = delete;

    ~Recorder();

    void start();

    void stop();

private:
    void setUpAlsa();
    void setHardwareParameters();
    void setSoftwareParameters();
    void tearDownAlsa();
    int capture();
    int recover(int err);

    const std::string deviceName_;
    const unsigned int sampleRate_;
    const unsigned int periodTime_;
    const unsigned int periodSize_;
    const unsigned int channels_;
    const Mode mode_;

    uint16_t sequenceNumber_;

    Transmitter& transmitter_;

    PacketPool& pool_;

    snd_pcm_t* pcm_;

    std::unique_ptr<std::thread> thread_;
    std::atomic_bool running_;
};

#endif  // __RECORDER_H
