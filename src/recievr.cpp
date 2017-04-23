// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#include "Player.h"
#include "Receiver.h"
#include "CircularBuffer.h"
#include "Utils.h"

#include <readerwriterqueue.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <atomic>
#include <unistd.h>
#include <signal.h>

using namespace boost::program_options;
using namespace moodycamel;

static const std::string DefaultDeviceName = "default";
static const unsigned int DefaultSampleRate = 48000;
static const unsigned int DefaultPeriodTime = 1000; // in microseconds
static const unsigned int DefaultChannels = 2;
static const unsigned int DefaultLatency = 10;
static const std::string DefaultAddress = "224.1.2.3";
static const unsigned int DefaultPort = 23776;

static void signalHandler(int) {
    static unsigned int count = 0;
    count++;
    if (count > 1) {
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    std::string deviceName = DefaultDeviceName;
    unsigned int sampleRate = DefaultSampleRate;
    unsigned int periodTime = DefaultPeriodTime;
    unsigned int channels = DefaultChannels;
    unsigned int latency = DefaultLatency;
    std::string address = DefaultAddress;
    unsigned short port = DefaultPort;
    bool verbose = false;

    options_description desc("Options");
    desc.add_options()
        ("device,d", value<std::string>(&deviceName)->default_value(DefaultDeviceName), "device name of the audio hardware")
        ("samplerate,s", value<unsigned int>(&sampleRate)->default_value(DefaultSampleRate), "sample rate in sample per second")
        ("periodtime,t", value<unsigned int>(&periodTime)->default_value(DefaultPeriodTime), "period time in microseconds (125, 250, 333, 1000)")
        ("channels,c", value<unsigned int>(&channels)->default_value(DefaultChannels), "number of channels")
        ("latency,l", value<unsigned int>(&latency)->default_value(DefaultLatency), "the fixed latency in milliseconds")
        ("address,a", value<std::string>(&address)->default_value(DefaultAddress), "destination address for the stream")
        ("port,p", value<unsigned short>(&port)->default_value(DefaultPort), "destination port for the stream")
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
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << desc << "\n";
        return -1;
    }

    if (verbose) {
        std::cout << "Receiving stream from " << address << ":" << port << "\n";
    }

    try {
        const auto periodSize = static_cast<unsigned int>(std::ceil(sampleRate * 0.000001 * periodTime));

        std::atomic<bool> streaming(false);
        ReaderWriterQueue<double> timeinfoQueue(10);
        CircularBuffer buffer(periodSize, channels, latency);

        Receiver receiver(address, port, sampleRate, periodTime, periodSize, 
            channels, latency, buffer, timeinfoQueue, streaming);
        Player player(deviceName, sampleRate, periodTime, channels, latency,
            buffer, timeinfoQueue, streaming);

        receiver.start();
        player.start();

        signal(SIGINT, signalHandler);
        pause();

        player.stop();
        receiver.stop();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
}
