#include "controllers/network/authentication-handler.hpp"
#include "controllers/network/network-controller.hpp"
#include <cstring>
#include "controllers/network/connection-data.hpp"
#include "controllers/network/message-templates.hpp"
#include "composites/network-node.hpp"
#include "controllers/network/VMAC-stream-hasher.hpp"
#include "controllers/network/ESIGN-signature.hpp"
#include "crypto++/vmac.h"
#include "trillek-game.hpp"
#include "logging.hpp"

namespace trillek { namespace network {

std::shared_ptr<chain_t> Authentication::auth_init_handler;
std::string Authentication::password;
CryptoPP::FixedSizeAlignedSecBlock<byte,16> Authentication::secret_key;

extern template void NetworkController::CloseConnection<CLIENT>(const Message* frame) const;
extern template	void NetworkController::CloseConnection<SERVER>(const Message* frame) const;

int Authentication::RetrieveSalt() const {
    auto req_list = TrillekGame::GetNetworkServer()
            .GetPacketHandler().GetQueue<NET_MSG,AUTH_INIT, SERVER>().Poll();
    if (req_list.empty()) {
        return STOP;
    }
    trillek_list<std::shared_ptr<Message>> temp_salt;
    for(auto& req : req_list) {
        if (req->CxData()->SetAuthState(AUTH_KEY_EXCHANGE)) {
            auto reply = make_unique<Message>(req->FileDescriptor());
            reply->Resize<NONE>(sizeof(SendSaltPacket));
            // TODO : salt is hardcoded !!!
            auto login = req->Content<AuthInitPacket>()->login;
            // GetSaltFromsomewhere(reply->Body(), packet->login);
            std::memcpy(reply->Content<SendSaltPacket,char>(), std::string("abcdefgh").data(), 8);
            LOGMSGC(DEBUG) << "Queing the salt ready to be sent to client";
            temp_salt.push_back(std::move(reply));
        }
    }
    salt_retrieved.PushList(std::move(temp_salt));
    if (salt_retrieved.Empty()) {
        return STOP;
    }
    return CONTINUE;
}


int Authentication::SendSalt() const {
    auto req_list = salt_retrieved.Poll();
    if (req_list.empty()) {
        return STOP;
    }
    NetworkController &server = TrillekGame::GetNetworkServer();
    for(auto& req : req_list) {
        auto reply = req->FrameHeader();
        if (reply) {
            // send salt
            req->SendMessageNoVMAC(req->fd, NET_MSG, AUTH_SEND_SALT);
        }
        else {
            // close connection
            // TODO: send error message
            //TCPConnection(req->fd, NETA_IPv4, SCS_CONNECTED).Send(reinterpret_cast<char*>(challenge.get()), sizeof(*challenge));
            LOGMSGC(WARNING) << "Authentication failed: User unknown";
            server.CloseConnection<SERVER>(req.get());
        }
    }
    return CONTINUE;
}

void Authentication::CheckKeyExchange(const trillek_list<std::shared_ptr<Message>>& req_list) {
    // server side
    NetworkController &server = TrillekGame::GetNetworkServer();
    for(auto& req : req_list) {
        if(! req->CxData()->CompareAuthState(AUTH_KEY_EXCHANGE)) {
            // This is not the packet expected
            LOGMSG(DEBUG) << "Unexpected packet received with length = " << req->PacketSize();
            server.CloseConnection<SERVER>(req.get());
            continue;
        }
        // Derive password and salt to get key
        // TODO : key should be stored and not be computed at runtime. The password is not on the server !
        // TODO : remove salt from KeyExchangePacket
        CryptoPP::FixedSizeAlignedSecBlock<byte,16> vkey;
        auto key = vkey.data();
        std::string password("secret password");
        auto recv_packet = req->Content<KeyExchangePacket>();
        Crypto::PBKDF(key,
             reinterpret_cast<const byte*>(password.data()), password.size(), recv_packet->salt, SALT_SIZE);
        if(! recv_packet->VerifyVMAC(key)) {
			LOGMSG(WARNING) << "Authentification failed: Bad password";
            req->CxData()->SetAuthState(AUTH_NONE);
            TrillekGame::GetNetworkServer().CloseConnection<SERVER>(req.get());
            continue;
        }
        // change state to SHARE_KEY and return the TCPConnection unique_ptr
        // TODO: replace by raknet
//        auto cnx = req->CxData()->ConnectionAccept();
        int cnx = 0;
        if (cnx) {
            auto checker_key = recv_packet->VMAC_BuildHasher(key);
            // TODO: Hard coded entity #id
            auto authentifier = std::allocate_shared<cryptography::VMAC_StreamHasher>
                                    (TrillekAllocator<cryptography::VMAC_StreamHasher>(),
                                     std::move(checker_key),
                                     recv_packet->nonce2, 8);
            auto cx_data = new ConnectionData(1, authentifier->Hasher(), authentifier->Verifier());
            Message reply_packet{};
            reply_packet << *server.ServerPublicKey();
            // TODO: Hardcoded entity
            id_t entity_id = 1;
            std::vector<unsigned char> eid(sizeof(id_t));
            std::memcpy(eid.data(), &entity_id, sizeof(id_t));
            reply_packet << eid;
			LOGMSG(DEBUG) << "Authentication OK";
            NetworkNode::AddEntity(entity_id, std::move(cnx));
/*
            server.Poller()->Create(
                    req->FileDescriptor(),reinterpret_cast<void*>(cx_data));
*/
            reply_packet.Send(req->fd, NET_MSG, AUTH_KEY_REPLY,
                    cx_data->Hasher(), reply_packet.Tail<unsigned char*>(),
                    VMAC_SIZE, VMAC_SIZE);
        }
    }
}

void Authentication::CreateSecureKey(const trillek_list<std::shared_ptr<Message>>& req_list) {
    // client side
    for(auto& req : req_list) {
        // We received reply
        req->RemoveVMACTag();
        auto msg = reinterpret_cast<unsigned char*>(req->FrameHeader());
        NetworkController &client = TrillekGame::GetNetworkClient();
        auto v = client.Verifier();
        if ((v)(req->Tail<const unsigned char*>(), msg, req->PacketSize()) && req->CxData()->SetAuthState(AUTH_SHARE_KEY)) {
            auto pkt = req->Content<KeyReplyPacket>();
            auto key = make_unique<std::vector<unsigned char>>(PUBLIC_KEY_SIZE);
            std::memcpy(key->data(), &pkt->challenge, PUBLIC_KEY_SIZE);
            client.SetEntityID(pkt->entity_id);
            //Authentication::GetNetworkSystem<CLIENT>()->SetServerPublicKey(std::move(key));
            auto esign = std::make_shared<cryptography::ESIGN_Signature>();
            esign->SetPublicKey(std::move(key));
            esign->Initialize();
            client.SetVerifier(esign->Verifier());
            client.SetAuthState(AUTH_SHARE_KEY);
			LOGMSG(DEBUG) << "Authentication OK";
            client.is_connected.notify_all();
        }
        else {
			LOGMSG(ERROR) << "Authentification failed: Could not authenticate the server";
            req->CxData()->SetAuthState(AUTH_NONE);
            client.SetAuthState(AUTH_NONE);
            client.CloseConnection<CLIENT>(req.get());
            client.is_connected.notify_all();
        }
    }
}

Message SendSaltPacket::GetKeyExchangePacket() {
    // Client received salt
    Message frame{};
    auto packet = frame.Content<KeyExchangePacket>();
    // Derive password and salt to get key
    Crypto::PBKDF(Authentication::GetSecretKey(),
            reinterpret_cast<const byte*>(Authentication::Password().data()),
            Authentication::Password().size(), salt, SALT_SIZE);
    std::memcpy(packet->salt, salt, SALT_SIZE);
    Crypto::GetRandom64(packet->nonce);
    Crypto::GetRandom128(packet->nonce2);
    Crypto::GetRandom128(packet->alea);
    Crypto::VMAC64(packet->vmac, frame.Content<KeyExchangePacket,const byte>(),
             VMAC_MSG_SIZE, Authentication::GetSecretKey(), packet->nonce);
    return std::move(frame);
}

bool KeyExchangePacket::VerifyVMAC(const byte* key) const {
    // server side
    return Crypto::VMAC_Verify(vmac, reinterpret_cast<const byte*>(this), VMAC_MSG_SIZE, key, nonce);
}

std::unique_ptr<CryptoPP::FixedSizeAlignedSecBlock<byte,16>> KeyExchangePacket::VMAC_BuildHasher(const byte* key) const {
    // server side
    // The variable that will hold the hasher key for this session
    auto checker_key = make_unique<CryptoPP::FixedSizeAlignedSecBlock<byte,16>>();
    // We derive the player key using the alea given by the player
    Crypto::VMAC128(checker_key->data(), alea, ALEA_SIZE, key, nonce2);
    return std::move(checker_key);
}

std::unique_ptr<CryptoPP::FixedSizeAlignedSecBlock<byte,16>> KeyExchangePacket::VMAC_BuildHasher() const {
    // client side
    // The variable that will hold the hasher key for this session
    auto checker_key = make_unique<CryptoPP::FixedSizeAlignedSecBlock<byte,16>>();
    // We derive the player key using the alea given by the player
    Crypto::VMAC128(checker_key->data(), alea, ALEA_SIZE, Authentication::GetSecretKey(), nonce2);
    return std::move(checker_key);
}

namespace packet_handler {
template<>
void PacketHandler::Process<NET_MSG,AUTH_INIT, SERVER>()  const {
    TrillekGame::GetScheduler().Execute(
        std::make_shared<TaskRequest<chain_t>>(Authentication::GetAuthInitHandler()));
}

template<>
void PacketHandler::Process<NET_MSG,AUTH_SEND_SALT, CLIENT>() const {
    auto req_list = GetQueue<NET_MSG,AUTH_SEND_SALT, CLIENT>().Poll();
    if (req_list.empty()) {
        return;
    }
    NetworkController &client = TrillekGame::GetNetworkClient();
    for(auto& req : req_list) {
        if (req->CxData()->SetAuthState(AUTH_KEY_EXCHANGE)) {
            // We must send keys
            auto frame = req->Content<SendSaltPacket>()->GetKeyExchangePacket();
            auto packet = frame.Content<KeyExchangePacket>();
            // TODO !!!!!
            auto hasher_key = std::move(packet->VMAC_BuildHasher());
            LOGMSG(DEBUG) << "Sending keys : " << frame.PacketSize()
                    << " bytes, key is " << std::hex << (uint64_t) hasher_key->data();
            auto authentifier = std::allocate_shared<cryptography::VMAC_StreamHasher>
                            (TrillekAllocator<cryptography::VMAC_StreamHasher>(),
                             std::move(hasher_key),
                             packet->nonce2, 8);
            client.SetHasher(authentifier->Hasher());
            client.SetVerifier(authentifier->Verifier());
            client.SetAuthState(AUTH_KEY_EXCHANGE);
            frame.SendMessageNoVMAC(req->fd, NET_MSG, AUTH_KEY_EXCHANGE);
        }
    }
}

template<>
void PacketHandler::Process<NET_MSG,AUTH_KEY_EXCHANGE, SERVER>() const {
    auto req_list = GetQueue<NET_MSG,AUTH_KEY_EXCHANGE, SERVER>().Poll();
    if (req_list.empty()) {
        return;
    }
    Authentication::CheckKeyExchange(req_list);
}

template<>
void PacketHandler::Process<NET_MSG,AUTH_KEY_REPLY, CLIENT>() const {
    auto req_list = GetQueue<NET_MSG,AUTH_KEY_REPLY, CLIENT>().Poll();
    if (req_list.empty()) {
        return;
    }
    Authentication::CreateSecureKey(req_list);
}
} // packet_handler
} // network

namespace reflection {
template <> inline const char* GetNetworkHandler<NET_MSG,AUTH_INIT>(void) {
    return "AuthenticationHandler";
}

template <> inline const char* GetNetworkHandler<NET_MSG,AUTH_KEY_EXCHANGE>(void) {
    return "AuthenticationHandler";
}
} // reflection
} // trillek
