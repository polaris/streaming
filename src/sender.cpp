// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#include "Recorder.h"
#include "Transmitter.h"
#include "PacketPool.h"
#include "Packet.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <unistd.h>

using namespace boost::program_options;

static const std::string DefaultDeviceName = "default";
static const unsigned int DefaultSampleRate = 48000;
static const unsigned int DefaultPeriodTime = 1000; // period time in microseconds
static const unsigned int DefaultChannels = 2;
static const std::string DefaultAddress = "224.1.2.3";
static const unsigned int DefaultPort = 23776;

int main(int argc, char* argv[]) {
    std::string deviceName = DefaultDeviceName;
    unsigned int sampleRate = DefaultSampleRate;
    unsigned int periodTime = DefaultPeriodTime;
    unsigned int channels = DefaultChannels;
    std::string address = DefaultAddress;
    unsigned short port = DefaultPort;
    bool verbose = false, click = false;

    options_description desc("Options");
    desc.add_options()
        ("device,d", value<std::string>(&deviceName)->default_value(DefaultDeviceName), "device name of the audio hardware")
        ("samplerate,s", value<unsigned int>(&sampleRate)->default_value(DefaultSampleRate), "sample rate in sample per second")
        ("periodtime,t", value<unsigned int>(&periodTime)->default_value(DefaultPeriodTime), "packet time in microseconds (125, 250, 333, 1000)")
        ("channels,c", value<unsigned int>(&channels)->default_value(DefaultChannels), "number of channels")
        ("address,a", value<std::string>(&address)->default_value(DefaultAddress), "destination address for the stream")
        ("port,p", value<unsigned short>(&port)->default_value(DefaultPort), "destination port for the stream")
        ("click,k", "generate click sound every second instead of capturing PCM from the audio interface")
        ("verbose,v", "verbose output")
        ("help,h", "produce help message");

    try {
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        verbose = vm.count("verbose") > 0;
        click = vm.count("click") > 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << desc << "\n";
        return -1;
    }

    if (verbose) {
        std::cout << "Streaming to " << address << ":" << port << " with " << sampleRate << "Hz, " << periodTime << "us per packet, " << channels << " channels\n";
    }

    try {
        const unsigned int payloadSize = static_cast<unsigned int>(std::round(sampleRate * 0.000001 * periodTime)) * channels * static_cast<unsigned int>(sizeof(int16_t));
        Recorder::Mode mode = click ? Recorder::Mode::Click : Recorder::Mode::Capture;

        PacketPool pool;
        for (int i = 0; i < 5; ++i) {
            pool.push(new Packet(payloadSize));
        }
        Transmitter transmitter(address.c_str(), port, pool);
        Recorder recorder(deviceName, sampleRate, periodTime, channels, mode, transmitter, pool);
        recorder.start();

        pause();

        recorder.stop();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
}
