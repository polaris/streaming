#ifndef __PACKETPOOL_H
#define __PACKETPOOL_H

#include <stack>
#include <mutex>

class Packet;

class PacketPool {
public:
    PacketPool()
    : mutex_()
    , stack_() {
    }

    void push(Packet* element) {
        std::unique_lock<std::mutex> lock(mutex_);
        stack_.push(element);
    }

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
    std::mutex mutex_;
    std::stack<Packet*> stack_;
};

#endif  // __PACKETPOOL_H
