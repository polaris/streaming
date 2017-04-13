#ifndef __PACKET_H
#define __PACKET_H

#include <cstdint>
#include <arpa/inet.h>

struct PacketHeader {
    uint16_t sequenceNumber;
    uint32_t timestamp;
} __attribute__((packed));

struct Packet {
    Packet(unsigned int dataSize)
    : dataSize_(dataSize)
    , packetSize_(dataSize_ + headerSize)
    , packet_(new uint8_t [packetSize_])
    , data_(packet_ + headerSize)
    , header_(reinterpret_cast<PacketHeader*>(packet_)) {
    }

    ~Packet() {
        delete [] packet_;
    }

    Packet(const Packet&) = delete;
    Packet& operator =(const Packet&) = delete;

    void setSequenceNumber(uint16_t sequenceNumber) {
        header_->sequenceNumber = htons(sequenceNumber);
    }
    uint16_t getSequenceNumber() {
        return ntohs(header_->sequenceNumber);
    }

    void setTimestamp(uint32_t timestamp) {
        header_->timestamp = htonl(timestamp);
    }
    uint32_t getTimestamp() {
        return ntohl(header_->timestamp);
    }

    static const uint32_t headerSize = 6;
    const uint32_t dataSize_;
    const uint32_t packetSize_;
    uint8_t* packet_;
    uint8_t* data_;
    PacketHeader* header_;
};

#endif  // __PACKET_H
