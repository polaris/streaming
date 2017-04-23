// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __PACKETPOOL_H
#define __PACKETPOOL_H

#include <stack>
#include <mutex>

class Packet;

/** A pool of packets.
 */
class PacketPool {
public:
    /** Constructor
     */
    PacketPool()
    : mutex_()
    , stack_() {
    }

    /** Pushes a packet into the pool.
     *
     *  \param element a pointer to the packet to be pushed.
     */
    void push(Packet* element) {
        std::unique_lock<std::mutex> lock(mutex_);
        stack_.push(element);
    }

    /** Returns a packet from the pool.
     *
     *  \return a pointer to a valid packet if the pool is non-empty, otherwise nullptr.
     */
    Packet* pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        Packet* result = nullptr;
        if (!stack_.empty()) {
            result = stack_.top();
            stack_.pop();
        }
        return result;
    }

private:
    std::mutex mutex_;          /**< A mutex to be protected by access by different threads. */
    std::stack<Packet*> stack_; /**< The internal stack of packets.                          */
};

#endif  // __PACKETPOOL_H
