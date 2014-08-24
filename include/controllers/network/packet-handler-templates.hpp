#ifndef PACKETHANDLERTEMPLATES_H_INCLUDED
#define PACKETHANDLERTEMPLATES_H_INCLUDED

namespace trillek { namespace network { namespace packet_handler {
extern template void PacketHandler::Process<NET_MSG,AUTH_INIT, SERVER>() const;
extern template void PacketHandler::Process<NET_MSG,AUTH_SEND_SALT, CLIENT>() const;
extern template void PacketHandler::Process<NET_MSG,AUTH_KEY_EXCHANGE, SERVER>() const;
extern template void PacketHandler::Process<NET_MSG,AUTH_KEY_REPLY, CLIENT>() const;
extern template void PacketHandler::Process<TEST_MSG,TEST_MSG, SERVER>() const;
extern template void PacketHandler::Process<TEST_MSG,TEST_MSG, CLIENT>() const;
} // packet_handler
} // network
} // trillek

#endif // PACKETHANDLERTEMPLATES_H_INCLUDED
