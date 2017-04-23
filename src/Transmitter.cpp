// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#include "Transmitter.h"
#include "PacketPool.h"
#include "Packet.h"

#include <iostream>

Transmitter::Transmitter(const char* address, unsigned short port, PacketPool& pool)
: endpoint_(boost::asio::ip::address::from_string(address), port)
, pool_(pool)
, service_()
, work_(service_)
, socket_(service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0))
, thread_(new std::thread([this] () { service_.run(); })) {
    socket_.non_blocking(true);
}

Transmitter::~Transmitter() {
    socket_.cancel();
    service_.stop();
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    thread_.reset();
    service_.reset();
}

void Transmitter::send(Packet* packet) {
    auto buffer = boost::asio::buffer(packet->packet_, packet->packetSize_);
    socket_.async_send_to(buffer, endpoint_,
        [this, packet] (const boost::system::error_code& ec, 
            std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "Failed to send packet: " << ec.message() << "\n";
            } else {
                assert(bytes_transferred == packet->packetSize_);
            }
            pool_.push(packet);
        });
}
