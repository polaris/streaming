#ifndef __TRANSMITTER_H
#define __TRANSMITTER_H

#include <boost/asio.hpp>

#include <thread>

class Packet;
class PacketPool;

class Transmitter {
public:
    Transmitter(const char* address, unsigned short port, PacketPool& pool);

    ~Transmitter();

    void send(Packet* packet);

private:
    const boost::asio::ip::udp::endpoint endpoint_;
	PacketPool& pool_;
    boost::asio::io_service service_;
    boost::asio::io_service::work work_;
    boost::asio::ip::udp::socket socket_;
    std::unique_ptr<std::thread> thread_;
};

#endif  // __TRANSMITTER_H
