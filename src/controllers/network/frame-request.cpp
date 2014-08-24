#include "controllers/network/frame-request.hpp"
#include "controllers/network/network-controller.hpp"
#include "trillek-game.hpp"

//TODO: Remove Visual Studio specific code
// when a decent std::chrono implementation will be available

namespace trillek {
namespace network {

// We disable VMAC check when template parameter is NONE
template<>
void Frame_req::CheckIntegrityTag<NONE>() const {};

template<>
void Frame_req::CheckIntegrityTag<CLIENT>() const {
    reassembled_frames_list.remove_if(
        [&](const std::shared_ptr<Message>& message) {
            auto size_to_check = message->PacketSize() - ESIGN_SIZE;
            message->RemoveTailClient();
            auto tail = message->Tail<msg_tail*>();
            return tail->entity_id !=
                TrillekGame::GetNetworkClient().EntityID() ||
                ! (TrillekGame::GetNetworkClient().Verifier())(
                    tail->tag, reinterpret_cast<uint8_t*>(message->FrameHeader()),
                    size_to_check);
        });
}

template<>
void Frame_req::CheckIntegrityTag<SERVER>() const {
    reassembled_frames_list.remove_if(
        [&](const std::shared_ptr<Message>& message) {
                 message->RemoveVMACTag();
                 return ! (cx_data->Verifier())(message->Tail<unsigned char*>(),
                                reinterpret_cast<uint8_t*>(message->FrameHeader()), message->PacketSize());
        });
}

Frame_req::Frame_req(const netport::socket_t fd, size_t length_total, const ConnectionData* const cxdata_ptr) :
    fd(fd), reassembled_frames_list(),
    length_total(length_total),
    length_requested(sizeof(Frame_hdr)),
    length_got(0),
    timeout(TIMEOUT),
#if defined(_MSC_VER)
    expiration_time(TrillekGame::GetOS().GetTime() + timeout),
#else
    expiration_time(std::chrono::steady_clock::now() + timeout),
#endif
    cx_data(cxdata_ptr) {
    reassembled_frames_list.emplace_back(new Message(fd, cxdata_ptr));
};

#if defined(_MSC_VER)
void Frame_req::UpdateTimestamp() { this->expiration_time = TrillekGame::GetOS().GetTime() + timeout; };
bool Frame_req::HasExpired() const { return (TrillekGame::GetOS().GetTime() > expiration_time) ; };
#else
void Frame_req::UpdateTimestamp() { this->expiration_time = std::chrono::steady_clock::now() + timeout; };
bool Frame_req::HasExpired() const { return (std::chrono::steady_clock::now() > expiration_time) ; };
#endif

} // network
} // trillek
