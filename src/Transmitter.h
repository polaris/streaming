// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __TRANSMITTER_H
#define __TRANSMITTER_H

#include <boost/asio.hpp>

#include <thread>

class Packet;
class PacketPool;

/** A class used to transmit packets of audio data.
 */
class Transmitter {
public:
    /** Constructor.
     *
     *  \param address the destination address.
     *  \param port the destination port.
     *  \param pool a pool of packets.
     */
    Transmitter(const char* address, unsigned short port, PacketPool& pool);

    /** Destructor.
     */
    ~Transmitter();

    /** Sends a packet.
     */
    void send(Packet* packet);

private:
    const boost::asio::ip::udp::endpoint endpoint_;     /**< The destination endpoint.      */
    PacketPool& pool_;                                  /**< The pool of packets.           */
    boost::asio::io_service service_;                   /**< The ASIO service object.       */
    boost::asio::io_service::work work_;                /**< Fake work for the service.     */
    boost::asio::ip::udp::socket socket_;               /**< The UDP socket.                */
    std::unique_ptr<std::thread> thread_;               /**< The thread for the service.    */
};

#endif  // __TRANSMITTER_H
