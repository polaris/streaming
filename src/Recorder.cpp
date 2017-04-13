#include "Recorder.h"
#include "Transmitter.h"
#include "Packet.h"
#include "PacketPool.h"
#include "Utils.h"
#include "DelayLockedLoop.h"

#include <iostream>
#include <cmath>

Recorder::Recorder(const std::string& deviceName, unsigned int sampleRate, unsigned int periodTime, unsigned int channels, Mode mode, Transmitter& transmitter, PacketPool& pool)
: deviceName_(deviceName)
, sampleRate_(sampleRate)
, periodTime_(periodTime)
, periodSize_(static_cast<unsigned int>(std::round(sampleRate_ * 0.000001 * periodTime_)))
, channels_(channels)
, mode_(mode)
, sequenceNumber_(0)
, transmitter_(transmitter)
, pool_(pool)
, pcm_(nullptr)
, thread_()
, running_(false) {
}

Recorder::~Recorder() {
    stop();
}

void Recorder::start() {
    stop();
    setUpAlsa();
    running_ = true;
    thread_.reset(new std::thread([this] () { capture(); }));    
}

void Recorder::stop() {
    running_ = false;
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    thread_.reset();
}

void Recorder::setUpAlsa() {
    tearDownAlsa();

    int err = 0;
    err = snd_pcm_open(&pcm_, deviceName_.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        std::cerr << "Failed to open device: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    snd_config_update_free_global();

    setHardwareParameters();
    setSoftwareParameters();

    snd_output_t* output;
    err = snd_output_stdio_attach(&output, stdout, 0);
    if (err < 0) {
        std::cerr << "Failed to attach stdout: " << snd_strerror(err) << "\n";
    }

    snd_pcm_dump(pcm_, output);
}

void Recorder::setHardwareParameters() {
    int err = 0;

    snd_pcm_hw_params_t* params = nullptr;
    err = snd_pcm_hw_params_malloc(&params);
    if (err < 0) {
        std::cerr << "Failed to allocate snd_pcm_hw_params_t: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_any(pcm_, params);
    if (err < 0) {
        std::cerr << "Failed to fill hardware params with configuration space for a PCM: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_access(pcm_, params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
    if (err < 0) {
        std::cerr << "Failed to set access type to SND_PCM_ACCESS_MMAP_INTERLEAVED: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_format(pcm_, params, SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
        std::cerr << "Failed to set format to SND_PCM_FORMAT_S16_LE: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_rate(pcm_, params, sampleRate_, 0);
    if (err < 0) {
        std::cerr << "Failed to set sample rate to " << sampleRate_ << ": " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_channels(pcm_, params, channels_);
    if (err < 0) {
        std::cerr << "Failed to set number of channels to " << channels_ << " : " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_period_size(pcm_, params, periodSize_, 0);
    if (err < 0) {
        std::cerr << "Failed to set period size to " << periodSize_ << " : " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_periods(pcm_, params, 2, 0);
    if (err < 0) {
        std::cerr << "Failed to set periods to 2: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    const auto bufferSize = periodSize_ * 2;
    err = snd_pcm_hw_params_set_buffer_size(pcm_, params, bufferSize);
    if (err < 0) {
        std::cerr << "Failed to set buffer size to " << bufferSize << ": " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params(pcm_, params);
    if (err < 0) {
        std::cerr << "Failed to install hardware configuration for PCM: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    snd_pcm_hw_params_free(params);
}

void Recorder::setSoftwareParameters() {
    int err = 0;

    snd_pcm_sw_params_t* params = nullptr;
    err = snd_pcm_sw_params_malloc(&params);
    if (err < 0) {
        std::cerr << "Failed to allocate snd_pcm_sw_params_t: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_sw_params_current(pcm_, params);
    if (err < 0) {
        std::cerr << "Failed to get current software configuration: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_sw_params_set_avail_min(pcm_, params, 0);
    if (err < 0) {
        std::cerr << "Failed to set avail min: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_sw_params_set_start_threshold(pcm_, params, 0);
    if (err < 0) {
        std::cerr << "Failed to set start threshold: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_sw_params_set_tstamp_mode(pcm_, params, SND_PCM_TSTAMP_ENABLE);
    if (err < 0) {
        std::cerr << "Failed to set tstamp mode to SND_PCM_TSTAMP_ENABLE: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_sw_params_set_tstamp_type(pcm_, params, SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY);
    if (err < 0) {
        std::cerr << "Failed to set tstamp type to SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    err = snd_pcm_sw_params(pcm_, params);
    if (err < 0) {
        std::cerr << "Failed to install software configuration for PCM: " << snd_strerror(err) << "\n";
        exit(EXIT_FAILURE);
    }

    snd_pcm_sw_params_free(params);
}

void Recorder::tearDownAlsa() {
    if (pcm_) {
        int err = snd_pcm_close(pcm_);
        if (err < 0) {
            std::cerr << "Failed to close PCM: " << snd_strerror(err) << "\n";
        }
        pcm_ = nullptr;
    }
}

int Recorder::capture() {
    static const double freq = 1760;
    static const double max_phase = 2. * M_PI;
    const double step = max_phase * freq / static_cast<double>(sampleRate_);
    double phase = 0;

    int err = 0, first = 1;

    snd_pcm_status_t *status = nullptr;
    snd_pcm_status_malloc(&status);

    snd_pcm_audio_tstamp_config_t audio_tstamp_config;
    audio_tstamp_config.type_requested = 0;
    audio_tstamp_config.report_delay = 1;
    snd_pcm_status_set_audio_htstamp_config(status, &audio_tstamp_config);

    DelayLockedLoop dll(periodTime_ * 0.000001);
    dll.reset(get_time());

    bool firstPeriod = true;
    uint32_t lastSample = 0, nextSample = 0;

    while (running_) {
        auto state = snd_pcm_state(pcm_);
        if (state == SND_PCM_STATE_XRUN) {
            std::cerr << "XRUN\n";
            err = recover(-EPIPE);
            if (err < 0) {
                std::cerr << "Failed to recover from XRUN: " << snd_strerror(err) << "\n";
                return err;
            }            
            first = 1;
        } else if (state == SND_PCM_STATE_SUSPENDED) {
            std::cerr << "SUSPENDED\n";
            err = recover(-ESTRPIPE);
            if (err < 0) {
                std::cerr << "Failed to recover from SUSPEND: " << snd_strerror(err) << "\n";
                return err;
            }            
        }

        err = snd_pcm_status(pcm_, status);
        if (err < 0) {
            std::cerr << "snd_pcm_status failed\n";
            err = recover(static_cast<int>(err));
            if (err < 0) {
                std::cerr << "Failed to update avail: " << snd_strerror(err) << "\n";
                return err;
            }            
            first = 1;
            continue;
        }

        snd_pcm_sframes_t avail = snd_pcm_status_get_avail(status);
        if (avail < static_cast<snd_pcm_sframes_t>(periodSize_)) {
            if (first) {
                first = 0;
                err = snd_pcm_start(pcm_);
                if (err < 0) {
                    std::cerr << "Failed to start: " << snd_strerror(err) << "\n";
                    return err;
                }
            } else {
                err = snd_pcm_wait(pcm_, -1);
                if (err < 0) {
                    std::cerr << "snd_pcm_wait failed\n";
                    if ((err = recover(err)) < 0) {
                        std::cerr << "Failed to wait for PCM: " << snd_strerror(err) << "\n";
                        return err;
                    }                    
                    first = 1;
                }
            }
            continue;
        }

        dll.update(get_time());
        uint32_t sample = static_cast<uint32_t>(static_cast<int64_t>(
                std::floor(dll.t0() * sampleRate_ + 0.5)
            ) % 4294967296);

        int32_t error = 0;
        if (firstPeriod == false) {
            auto diff = sample - lastSample;
            if (diff != periodSize_) {
                error = periodSize_ - diff;
            }
            if (nextSample - error != sample) {
                std::cout << sample << " " << nextSample << "\n";
            }
        }
        firstPeriod = false;
        lastSample = sample;

        snd_pcm_uframes_t size = periodSize_;
        while (size > 0) {
            snd_pcm_uframes_t offset = 0, frames = size;
            const snd_pcm_channel_area_t* channel_area = nullptr;
            err = snd_pcm_mmap_begin(pcm_, &channel_area, &offset, &frames);
            if (err < 0) {
                std::cerr << "snd_pcm_mmap_begin failed\n";
                if ((err = recover(err)) < 0) {
                    std::cerr << "Failed MMAP begin: " << snd_strerror(err) << "\n";
                    return err;
                }                
                first = 1;
            }

            Packet* packet = pool_.pop();
            if (packet != nullptr) {
                packet->setSequenceNumber(sequenceNumber_++);
                packet->setTimestamp(sample + error);
                auto data = reinterpret_cast<int16_t*>(packet->data_);
                if (mode_ == Mode::Click) {
                    for (unsigned int frame = 0; frame < frames; frame++) {
                        int16_t value = 0;
                        if ((sample + frame) % sampleRate_ < 1000) {
                            value = static_cast<int16_t>(sin(phase) * (.5 * 0x8000));
                            phase += step;
                            if (phase >= max_phase) {
                                phase -= max_phase;
                            }
                        } else {
                            phase = 0;
                        }
                        for (unsigned int channel = 0; channel < channels_; channel++) {
                            *data = value;
                            data += 1;
                        }
                    }
                } else if (mode_ == Mode::Capture) {
                    for (unsigned int frame = 0; frame < frames; frame++) {
                        for (unsigned int channel = 0; channel < channels_; channel++) {
                            auto sampleAddress = (int16_t*)channel_area[channel].addr;
                            sampleAddress += (channel_area[channel].first + (frame + offset) * channel_area[channel].step) / 16;
                            *data = *sampleAddress;
                            data += 1;
                        }
                    }
                }

                nextSample = sample + periodSize_;

                transmitter_.send(packet);
            } else {
                std::cerr << "out of buffers\n";
            }

            snd_pcm_sframes_t commit_result = snd_pcm_mmap_commit(pcm_, offset, frames);
            if (commit_result < 0 || (snd_pcm_uframes_t)commit_result != frames) {
                std::cerr << "snd_pcm_mmap_commit failed\n";
                if ((err = recover(commit_result >= 0 ? -EPIPE : static_cast<int>(commit_result))) < 0) {
                    std::cerr << "Failed MMAP commit: " << snd_strerror(err) << "\n";
                    return err;
                }                
                first = 1;
            }

            sample += static_cast<uint32_t>(frames);

            size -= frames;
        }
    }

    snd_pcm_status_free(status);

    return 0;
}

int Recorder::recover(int err) {
    return snd_pcm_recover(pcm_, err, 1);
}
