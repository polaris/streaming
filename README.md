# streaming

A prototypical streaming system with focus on synchronized playback on multiple receivers.

Compilation of the code requires a C++ compiler that supports the C++11 standard (e. g. GCC 4.9.2 or newer). The development packages of a few libraries have to be installed to compile the code:

- alsa-lib 1.0.28 or newer
- libsamplerate 0.1.8 or newer
- Boost 1.55 or newer

The necessary libraries are usually available for installation using the packet manager of a Linux distribution. On Linux distribution based on Debian or Ubuntu the libraries can be installed using the program `apt-get` as follows: 

    $ sudo apt-get install libasound2-dev
    $ sudo apt-get install libsamplerate0-dev
    $ sudo apt-get install libboost-dev

The compilation is started by calling `make` on the command line. If a specific program has to be built `make sender` respective `make sender` can be used. Calling `make clean` removes previous build artifacts.
