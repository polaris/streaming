OBJDIR := obj

CFLAGS := -O3 -Wall -pedantic -pedantic-errors -Wextra -Wcast-align -Wcast-qual -Wchar-subscripts \
	-Wcomment -Wconversion -Wdisabled-optimization -Wfloat-equal -Wformat -Wformat=2 \
	-Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport -Winit-self -Winvalid-pch \
	-Wmissing-braces -Wmissing-field-initializers -Wmissing-format-attribute \
	-Wmissing-include-dirs -Wmissing-noreturn -Wparentheses -Wpointer-arith -Wredundant-decls \
	-Wreturn-type -Wsequence-point -Wshadow -Wsign-compare -Wstack-protector -Wswitch \
	-Wswitch-default -Wswitch-enum -Wtrigraphs -Wuninitialized -Wunknown-pragmas \
	-Wunreachable-code -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value \
	-Wunused-variable -Wvariadic-macros -Wvolatile-register-var -Wwrite-strings

LDFLAGS := -lboost_system -lboost_program_options -lasound -lm -lstdc++ -lsamplerate -isystem src/rwq -pthread -std=c++11

all: sender receiver

sender: src/sender.cpp src/Transmitter.cpp src/Transmitter.h src/Recorder.cpp src/Recorder.h src/Packet.h \
	    src/PacketPool.h src/Utils.cpp src/Utils.h src/DelayLockedLoop.h
	$(CC) $(CFLAGS) src/sender.cpp src/Transmitter.cpp src/Recorder.cpp src/Utils.cpp $(LDFLAGS) -o $@

receiver: src/recievr.cpp src/PacketPool.h src/Receiver.cpp src/Receiver.h src/Player.cpp src/Player.h \
		  src/Packet.h src/CircularBuffer.h src/Utils.cpp src/Utils.h src/DelayLockedLoop.h \
		  src/ResampleRatioEstimator.h src/Resampler.h
	$(CC) $(CFLAGS) src/recievr.cpp src/Receiver.cpp src/Player.cpp src/Utils.cpp $(LDFLAGS) -o $@

.PHONY: clean
clean:
	@rm -f *.o sender receiver
