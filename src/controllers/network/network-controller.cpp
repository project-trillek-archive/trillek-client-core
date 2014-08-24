#include "controllers/network/network-controller.hpp"

#include <chrono>
#include <cstring>
#include <sys/event.h>
#include "net/network.h"
#include <typeinfo>
#include "composites/network-node.hpp"
#include "controllers/network/ESIGN-signature.hpp"
#include "controllers/network/network-controller-templates.hpp"
#include "trillek-game.hpp"

using namespace network;

namespace trillek {
namespace network {

TCPConnection NetworkController::server_socket;

NetworkController::NetworkController() {};

void NetworkController::Initialize() throw (std::runtime_error) {
    if (! poller.Initialize()) {
        throw std::runtime_error("Could not initialize kqueue !");
    }
};

bool NetworkController::Server_Start(const std::string& host, unsigned short port) {
    auto esign_hasher = std::make_shared<cryptography::ESIGN_Signature>();
    esign_hasher->GenerateKeys();
    esign_hasher->Initialize();
    SetServerPublicKey(esign_hasher->PublicKey());
    SetHasher(esign_hasher->Hasher());

    // start to listen
    auto s = Listener(host, port);
    if (! s) {
        std::cout << "Failed to start NetworkSystem." << std::endl;
        return false;
    }
    server_handle = s.get_handle();
    server_socket = std::move(s);
    poller.CreatePermanent(server_socket.get_handle());
    auto tr = std::make_shared<TaskRequest<chain_t>>(handle_events);
    TrillekGame::GetScheduler().Queue(tr);
    return true;
}

TCPConnection NetworkController::Listener(const std::string& host, uint16_t port) {
    TCPConnection server;
    if (! server.init(NETA_IPv4)) {
//			LOG_ERROR << "Could not open socket !";
        return {};
    }

    NetworkAddress address(host, port);
    if (! server.bind(address)) {
//			LOG_ERROR << "Could not bind address !";
        return {};
    }

    if (! server.listen(5)) {
//			LOG_ERROR << "Could not listen on port !";
        return {};
    }
    return server;
}

bool NetworkController::Connect(const std::string& host, uint16_t port,
                        const std::string& login, const std::string& password) {
    auto tr = std::make_shared<TaskRequest<chain_t>>(handle_events);
    TrillekGame::GetScheduler().Queue(tr);

    authentication.SetPassword(password);

    cnx = TCPConnection();
    NetworkAddress address(host, port);
    if (! cnx.init(address)) {
        return false;
    }

    if (! cnx.connect(address)) {
        return false;
    }
    auto fd = cnx.get_handle();
    poller.Create(fd, reinterpret_cast<void*>(new ConnectionData(AUTH_INIT, TCPConnection())));

    Message packet{};
    std::strncpy(packet.Content<AuthInitPacket, char>(), login.c_str(), LOGIN_FIELD_SIZE - 1);
    packet.SendMessageNoVMAC(fd, NET_MSG, AUTH_INIT);
    SetAuthState(AUTH_INIT);
    std::unique_lock<std::mutex> locker(connecting);
    while (AuthState() != AUTH_SHARE_KEY && AuthState() != AUTH_NONE) {
        is_connected.wait_for(locker, std::chrono::seconds(5),
            [&]() {
                return (AuthState() == AUTH_SHARE_KEY || AuthState() == AUTH_NONE);
            });
    }
    if (AuthState() == AUTH_SHARE_KEY) {
        return true;
    }
    return false;
}

// This function is the 1st element of the handle_events chain.
int NetworkController::HandleEvents() const {
    trillek_list<std::shared_ptr<Frame_req>> temp_public;
    trillek_list<std::shared_ptr<Frame_req>> temp_auth;
    std::vector<struct kevent> evList(EVENT_LIST_SIZE);
    auto nev = Poller()->Poll(evList);
    if (nev < 0) {
        std::cout << "(" << sched_getcpu() << ") Error when polling event" << std::endl;
    }
    bool a = false, b = false;
    for (auto i=0; i<nev; i++) {
        auto fd = evList[i].ident;
        if (evList[i].flags & EV_EOF) {
            // connection closed
            if(evList[i].udata) {
                CloseConnection(fd, reinterpret_cast<ConnectionData*>(evList[i].udata));
            }
            else {
                RemoveConnection(fd);
            }
            continue;
        }
        if (evList[i].flags & EV_ERROR) {   /* report any error */
            std::cout << "EV_ERROR: " << evList[i].data << std::endl;
            continue;
        }

        if (fd == server_handle) {
            // new connection
            auto client = server_socket.accept();
            client.set_nonblocking(true);
            auto chandle = client.get_handle();
            poller.Create(chandle, reinterpret_cast<void*>(new ConnectionData(AUTH_INIT, std::move(client))));
        }
        else if (evList[i].flags & EVFILT_READ) {
            // Data received
            // we retrieve the ConnectionData instance
            auto cx_data = reinterpret_cast<ConnectionData*>(evList[i].udata);
            if (! cx_data || ! cx_data->TryLockConnection()) {
                // no instance or another thread is already on this socket ? leave
                continue;
            }
            // now the socket is locked to the current thread
            if (! cx_data->CompareAuthState(AUTH_SHARE_KEY)) {
                // Data received, not authenticated
                // Request the frame header
                // queue the task to reassemble the frame
                auto max_size = std::min(evList[i].data, static_cast<intptr_t>(MAX_UNAUTHENTICATED_FRAME_SIZE));
                auto f = std::allocate_shared<Frame_req,TrillekAllocator<Frame_req>>
                                        (TrillekAllocator<Frame_req>(),fd, max_size, cx_data);
                temp_public.push_back(std::move(f));
                a = true;
            }
            else {
                // Data received from authenticated client
                auto max_size = std::min(evList[i].data, static_cast<intptr_t>(MAX_AUTHENTICATED_FRAME_SIZE));
                auto f = std::allocate_shared<Frame_req,TrillekAllocator<Frame_req>>
                                        (TrillekAllocator<Frame_req>(),fd, max_size, cx_data);
                temp_auth.push_back(std::move(f));
                b = true;
            }
        }
    }
    if(a) {
        // if we got data from unauthenticated clients, push the public reassemble task
        GetPublicRawFrameReqQueue()->PushList(std::move(temp_public));
        TrillekGame::GetScheduler().Queue(std::make_shared<TaskRequest<chain_t>>(unauthenticated_recv_data));
    }
    if(b) {
        // if we got data from authenticated clients, push the data
        GetAuthenticatedRawFrameReqQueue()->PushList(std::move(temp_auth));
        // continue on private reassemble, and requeue the present block
        return SPLIT;
    }
    else {
        // Queue a task to wait another event
        return REQUEUE;
    }
}

void NetworkController::RemoveClientConnection() const {
    auto fd = cnx.get_handle();
    poller.Delete(fd);
    network::close(fd);
    SetAuthState(AUTH_NONE);
    is_connected.notify_all();
}

void NetworkController::RemoveConnection(socket_t fd) const {
    poller.Delete(fd);
    std::cout << "closing" << std::endl;
    network::close(fd);
    SetAuthState(AUTH_NONE);
    is_connected.notify_all();
}

void NetworkController::CloseConnection(const socket_t fd, const ConnectionData* cx_data) const {
    RemoveConnection(fd);
    NetworkNode::RemoveEntity(cx_data->Id());
    delete cx_data;
}
} // network
} // trillek
