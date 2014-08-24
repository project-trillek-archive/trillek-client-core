#include "controllers/network/test-packet-handler.hpp"

#include <iostream>
#include "composites/network-node.hpp"
#include "controllers/network/packet-handler.hpp"
#include "controllers/network/message.hpp"

namespace trillek { namespace network { namespace packet_handler {
template<>
void PacketHandler::Process<TEST_MSG,TEST_MSG, SERVER>() const {
    auto req_list = GetQueue<TEST_MSG,TEST_MSG, SERVER>().Poll();
    if (req_list.empty()) {
        return;
    }
    for(auto& req : req_list) {
//				LOG_DEBUG << "Received authenticated test message with content: " << std::string(req->Content<TestPacket>()->message) << " from id #" << req->GetId();
        Message packet{};
        packet << std::string(req->Content<TestPacket>()->message);
        packet.Send(req->GetId(), TEST_MSG, TEST_MSG);
    }
}

template<>
void PacketHandler::Process<TEST_MSG,TEST_MSG, CLIENT>() const {
/*    auto req_list = GetQueue<TEST_MSG,TEST_MSG, CLIENT>().Poll();
    if (req_list.empty()) {
        return;
    }
    for(auto& req : req_list) {
        std::cout << "Received authenticated test message with content: " << std::string(req->Content<TestPacket>()->message) << std::endl;
    }
*/}
} // packet_handler
} // network

namespace reflection {
template <> inline constexpr const char* GetNetworkHandler<TEST_MSG,TEST_MSG>(void) { return "TestPacketHandler"; }
}

} // trillek
