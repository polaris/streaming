// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __PACKET_H
#define __PACKET_H

#include <cstdint>
#include <arpa/inet.h>

/** The header of the packet.
 */
struct PacketHeader {
    uint32_t timestamp;
} __attribute__((packed));

/** A packet used to send unencoded audio data.
 */
struct Packet {
    /** Constructor
     *
     *  \param dataSize the size of the data payload.
     */
    Packet(unsigned int dataSize)
    : dataSize_(dataSize)
    , packetSize_(dataSize_ + headerSize)
    , packet_(new uint8_t [packetSize_])
    , data_(packet_ + headerSize)
    , header_(reinterpret_cast<PacketHeader*>(packet_)) {
    }

    /** Destructor
     */
    ~Packet() {
        delete [] packet_;
    }

    Packet(const Packet&) = delete;
    Packet& operator =(const Packet&) = delete;

    /** Sets the timestamp.
     *
     *  \param timestamp the timestamp to set.
     */
    void setTimestamp(uint32_t timestamp) {
        header_->timestamp = htonl(timestamp);
    }

    /** Returns the timestamp.
     */
    uint32_t getTimestamp() {
        return ntohl(header_->timestamp);
    }

    static const uint32_t headerSize = 4;       /**< The header size in bytes.                  */
    const uint32_t dataSize_;                   /**< The size of the data payload in bytes.     */
    const uint32_t packetSize_;                 /**< The total packet size in bytes.            */
    uint8_t* packet_;                           /**< A pointer to the packet.                   */
    uint8_t* data_;                             /**< A pointer to the data payload.             */
    PacketHeader* header_;                      /**< A pointer to the packet header.            */
};

#endif  // __PACKET_H
