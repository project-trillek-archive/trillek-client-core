#ifndef PACKETHANDLER_HPP_INCLUDED
#define PACKETHANDLER_HPP_INCLUDED

#include "atomic-queue.hpp"
#include "controllers/network/network.hpp"

namespace trillek { namespace network {

class Message;

namespace packet_handler {

template<int Major,int Minor, TagType T>
class PacketQueue {
public:
    static AtomicQueue<std::shared_ptr<Message>> input_queue;
};

template<int Major,int Minor, TagType T>
AtomicQueue<std::shared_ptr<Message>> PacketQueue<Major,Minor, T>::input_queue;

class PacketHandler {
public:
    PacketHandler() {};
    virtual ~PacketHandler() {};

    template<int Major, int Minor, TagType T>
    void Process() const;

    template<int Major, int Minor, TagType T>
    AtomicQueue<std::shared_ptr<Message>>& GetQueue() {
        return PacketQueue<Major,Minor, T>::input_queue;
    };

    template<int Major, int Minor, TagType T>
    AtomicQueue<std::shared_ptr<Message>>& GetQueue() const {
        return PacketQueue<Major,Minor, T>::input_queue;
    };

};
} // namespace packet_handler
} // namespace network
} // namespace trillek

#endif // PACKETHANDLER_HPP_INCLUDED
