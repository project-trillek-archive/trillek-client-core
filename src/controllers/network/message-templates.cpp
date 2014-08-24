#include "controllers/network/message.hpp"

namespace trillek { namespace network {
template<>
void Message::Resize<CLIENT>(size_t new_size) {
    packet_size = new_size + sizeof(Frame);
    data.resize(packet_size + VMAC_SIZE);
};

template<>
void Message::Resize<SERVER>(size_t new_size) {
    packet_size = new_size + sizeof(Frame);
    data.resize(packet_size + sizeof(msg_tail));
}

template<>
void Message::Resize<NONE>(size_t new_size) {
    packet_size = new_size + sizeof(Frame);
    data.resize(packet_size);
}
}
}