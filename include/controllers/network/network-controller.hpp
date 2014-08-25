#ifndef NETWORKCONTROLLER_H_INCLUDED
#define NETWORKCONTROLLER_H_INCLUDED

#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <algorithm>
#include "trillek.hpp"
#include "controllers/network/network.hpp"
#include "atomic-queue.hpp"
#include "controllers/network/message.hpp"
#include "controllers/network/authentication-handler.hpp"
#include "controllers/network/frame-request.hpp"
#include "controllers/network/packet-handler-templates.hpp"

// the maximum number of bytes we can receive
#define MAX_MESSAGE_SIZE    65535

// the number of events we get at each call of the event handler
#define EVENT_LIST_SIZE     32

// the maximum number of bytes we will process from each authenticated socket
// at each call of the event handler
#define MAX_AUTHENTICATED_FRAME_SIZE      1460L

// the maximum number of bytes we will process from each unauthenticated socket
// at each call of the event handler
#define MAX_UNAUTHENTICATED_FRAME_SIZE      128L

namespace network {
class TCPConnection;
};

namespace trillek { namespace network {

class IOPoller;
class GetSaltTaskRequest;

namespace packet_handler {
class PacketHandler;
}

class NetworkController {

public:
    friend void packet_handler::PacketHandler::Process<NET_MSG,AUTH_SEND_SALT, CLIENT>() const;
    friend int Authentication::SendSalt() const;
    friend void Authentication::CheckKeyExchange(const trillek_list<std::shared_ptr<Message>>& req_list);
    friend void Authentication::CreateSecureKey(const trillek_list<std::shared_ptr<Message>>& req_list);
    friend void Message::Send(unsigned char major, unsigned char minor);
    friend void Message::Send(id_t, unsigned char major, unsigned char minor);
    friend void Frame_req::CheckIntegrityTag<CLIENT>() const;

    NetworkController();
    virtual ~NetworkController() {};

    /** \brief Start the server to accept players
     *
     * \param host const std::string& the host to bind
     *
     */
    bool Server_Start(const std::string& host, unsigned short port);


    /** \brief Connect a client to a server
     *
     * \param host const std::string& the host to connect to
     * \param port the port to connect to
     * \param login the login to use
     * \param password the password to use
     * \return bool true if connected, false otherwise
     *
     */
    bool Connect(const std::string& host, unsigned short port,
                 const std::string& login, const std::string& password);

    /** \brief Initialize the TCP handler as a client or a server
     *
     * T is the mode type (client, server, etc.)
     */
    template<TagType T>
    void SetTCPHandler() {

        authentication.Initialize();

        handle_events = chain_t({
            std::bind(&NetworkController::HandleEvents, std::cref(*this)),
            // Get the length
            std::bind(&NetworkController::ReassembleFrame<T>, std::cref(*this), &auth_rawframe_req, &auth_checked_frame_req),
            std::bind(&NetworkController::AuthenticatedDispatch<T>, std::cref(*this))
        });

        unauthenticated_recv_data = chain_t({
            // Get the length
            std::bind(&NetworkController::ReassembleFrame<T, NONE>, std::cref(*this), &pub_rawframe_req, &pub_frame_req),
            std::bind(&NetworkController::UnauthenticatedDispatch<T>, std::cref(*this))
        });
    }

    /** \brief Set the public key that will be used to authenticate packets
     * received from the server.
     *
     * This function is used by the client when it receives the key from the server
     *
     * \param key the public key (32 bytes)
     */
    void SetServerPublicKey(std::vector<unsigned char>&& key) {
        serverPublicKey = std::move(key);
    };

    /** \brief Return the instance of the packet handler
     *
     * \return const network_packet_handler::PacketHandler& the instance
     *
     */
    const packet_handler::PacketHandler& GetPacketHandler() const {
        return packet_handler;
    };

private:
    /** \brief The listener waits for connections and pass new connections
     * to the IncomingConnection
     *
     * \param host const std::string& the host to bind
     *
     */
//    netport::TCPConnection Listener(const std::string& host, unsigned short port);

    /** \brief Set the hasher functor that will be used to add a tag to each packet sent
     *
     * \param hasher the hasher functor
     */
    void SetHasher(std::function<void(unsigned char*,const unsigned char*,size_t)>&& hasher) {
        this->hasher = std::move(hasher);
    };

    /** \brief Return the hasher functor used to add a tag to each packet sent
     *
     * \return the hasher functor
     */
    std::function<void(unsigned char*,const unsigned char*,size_t)>& Hasher() {
        return hasher;
    };

    /** \brief Set the verifier functor that will be used to check the tag of each packet received
     *
     * \param verifier the verifier functor
     */
    void SetVerifier(std::function<bool(const unsigned char*,const unsigned char*,size_t)>&& verifier) {
        this->verifier = std::move(verifier);
    };

    /** \brief Return the verifier functor used to check the tag of each packet received
     *
     * \return the verifier functor
     */
    std::function<bool(const unsigned char*,const unsigned char*,size_t)>& Verifier() { return verifier; };

    /** \brief Set the authentication state
     *
     * This function is used by the client
     *
     * \param state the state
     *
     */
    void SetAuthState(uint32_t state) const { auth_state.store(state); };

    /** \brief Get the authentication state
     *
     * \return uint32_t the state
     *
     */
    uint32_t AuthState() const { return auth_state.load(); };

    const AtomicQueue<std::shared_ptr<Frame_req>>* const GetAuthenticatedRawFrameReqQueue() const { return &auth_rawframe_req; };
    const AtomicQueue<std::shared_ptr<Message>>* const GetAuthenticatedCheckedFrameQueue() const { return &auth_checked_frame_req; };
    const AtomicQueue<std::shared_ptr<Frame_req>>* const GetPublicRawFrameReqQueue() const { return &pub_rawframe_req; };
    const AtomicQueue<std::shared_ptr<Message>>* const GetPublicReassembledFrameQueue() const { return &pub_frame_req; };

    /** \brief Handle the network events in a non-blocking way
     *
     * \return int return code for the scheduler
     *
     */
    int HandleEvents() const;

    /** \brief Close a connection through a Message
     *
     * Specialized in Networktemplates.cpp
     * T is the mode type (client, server, etc.)
     *
     * \param frame const Message*
     * \return void
     *
     */
    template<TagType T>
    void CloseConnection(const Message* frame) const;

    /** \brief Dispatch the unauthenticated frames to the packet handlers
     *
     * Specialized in Networktemplates.cpp
     * T is the mode type (client, server, etc.)
     *
     * \return int return code for the scheduler
     *
     */
    template<TagType T>
    int UnauthenticatedDispatch() const;

    /** \brief Dispatch the authenticated frames to the packet handlers
     *
     * Specialized in Networktemplates.cpp
     * T is the mode type (client, server, etc.)
     *
     * \return int return code for the scheduler
     *
     */
    template<TagType T>
    int AuthenticatedDispatch() const;

    /** \brief Close the connection
     *
     * This function is valid only on client side
     *
     */
    void RemoveClientConnection() const;

    /** \brief Close a connection and remove the network node component (server side)
     *
     * \param fd const int the file descriptor
     * \param cx_data const ConnectionData* the data containing the id of the entity
     * \return void
     *
     */
    void CloseConnection(const int fd, const ConnectionData* cx_data) const;


    /** \brief Close an uncomplete connection, i.e known by the network but not the game
     *
     * \param fd const int the file descriptor
     *
     */
    void RemoveConnection(const int fd) const;

    /** \brief Return the public key used to check packets received from the server
     *
     * To be used only on client side
     *
     * \return const std::vector<unsigned char>* the key
     *
     */
    const std::vector<unsigned char>* ServerPublicKey() const { return &serverPublicKey; };

    /** \brief Set the entity ID given by the server
     *
     * Used only by the client
     *
     * \param eid the entity ID given as a vector
     *
     */
    void SetEntityID(id_t eid) { entity_id = eid; };

    /** \brief Return the entity ID given by the server
     *
     * To be used only on the client side
     *
     * \return const std::vector<unsigned char>* the entity id
     *
     */
    id_t EntityID() const { return entity_id; };

    // instance of the authentication handler
    const Authentication authentication;

    // instance of the packet_handler
    const packet_handler::PacketHandler packet_handler;

    // chain of block functions
    chain_t unauthenticated_recv_data;
    chain_t handle_events;

    // The queues that will contain the work to do for each block
    const AtomicQueue<std::shared_ptr<Frame_req>> auth_rawframe_req;				// raw frame request, to be authenticated
    const AtomicQueue<std::shared_ptr<Message>> auth_checked_frame_req;		// reassembled frame request, authenticated
    const AtomicQueue<std::shared_ptr<Frame_req>> pub_rawframe_req;				// raw frame requests
    const AtomicQueue<std::shared_ptr<Message>> pub_frame_req;				// reassembled frame request

    std::vector<unsigned char> serverPublicKey;
    std::function<bool(const unsigned char*,const unsigned char*,size_t)> verifier;
    std::function<void(unsigned char*,const unsigned char*,size_t)> hasher;

    // members for client only
    mutable std::atomic_uint_least32_t auth_state;
    // used to observe the connection process result from another thread
    mutable std::condition_variable is_connected;
    // mutex associated with the unique_lock
    std::mutex connecting;
    // the entity ID provided by the server
    id_t entity_id;

    /** \brief Process the frame requests of input and put the reassembled messages in output
     *
     * T is the mode type (client, server, etc.)
     * checkAuth is the authentification tag to check, by default same as T
     *
     * The input may still have uncomplete frame requests when the function returns.
     *
     * \param input const AtomicQueue<std::shared_ptr<Frame_req>>*const the input queue
     * \param output const AtomicQueue<std::shared_ptr<Message>>*const the output queue
     * \return int return code
     *
     */
    template<TagType T, TagType checkAuth = T>
    int ReassembleFrame(const AtomicQueue<std::shared_ptr<Frame_req>>* const input, const AtomicQueue<std::shared_ptr<Message>>* const output) const {
        auto req_list = input->Poll();
        trillek_list<std::shared_ptr<Frame_req>> temp_input;
        trillek_list<std::shared_ptr<Message>> temp_output;
        if (req_list.empty()) {
            return STOP;
        }
        auto ret = CONTINUE;
//			LOG_DEBUG << "(" << sched_getcpu() << ") got " << req_list->size() << " Reassemble events";
        for (auto& req : req_list) {
            // Remember that each socket is locked to the current thread
            auto target_size = req->length_requested;
//				LOG_DEBUG << "(" << sched_getcpu() << ")  Reassembling " << target_size << " bytes";
            auto frame = req->reassembled_frames_list.back().get();
            char* buffer = reinterpret_cast<char*>(frame->FrameHeader());

            auto current_size = req->length_got;
            int len;
// TODO: replace by raknet
//            len = network::recv(req->fd, reinterpret_cast<char*>(buffer) + current_size, target_size - current_size);
            if(len < 0) {
//					LOG_ERROR << "(" << sched_getcpu() << ") Could not read data";
                continue;
            }

            if (len > MAX_MESSAGE_SIZE) {
//					LOG_DEBUG << "(" << sched_getcpu() << ") Packet length exceeding MAX_MESSAGE_SIZE bytes. closing";
                CloseConnection<T>(frame);
                continue;
            }
            current_size += len;
            req->length_got = current_size;

            if (current_size == sizeof(Frame_hdr) && target_size == sizeof(Frame_hdr)) {
                // We now have the length
                auto length = frame->FrameHeader()->length;
                target_size = frame->FrameHeader()->length + sizeof(Frame_hdr);
//					LOG_DEBUG << "(" << sched_getcpu() << ")  Completing message to " << target_size << " bytes in a frame of " << req->length_total << " bytes";
                req->length_requested = target_size;
                frame->Resize<NONE>(length - sizeof(msg_hdr));
                buffer = reinterpret_cast<char*>(frame->FrameHeader());
// TODO: replace by raknet
//                len = network::recv(req->fd, reinterpret_cast<char*>(buffer) + current_size, target_size - current_size);
                if (len < 0) {
//						LOG_ERROR << "(" << sched_getcpu() << ") Could not read data";
                    continue;
                }
                current_size += len;
                req->length_got = current_size;
            }

            if (current_size < target_size) {
                // missing bytes
                if (req->HasExpired()) {
                    // Frame reassembly is stopped after 3 seconds if message is uncomplete
//						LOG_DEBUG << "(" << sched_getcpu() << ") Dropping all frames and closing (timeout)";
                    CloseConnection<T>(frame);
                    continue;
                }
                // we put again the request in the queue
//					LOG_DEBUG << "(" << sched_getcpu() << ") got only " << current_size << " bytes, waiting " << target_size << " bytes";
                temp_input.push_back(std::move(req));
                if (ret == CONTINUE) {
                    ret = REPEAT;
                }
            }
            else {
                if ( current_size < req->length_total ) {
                    // message retrieved, but there are still bytes to read
//						LOG_DEBUG << "(" << sched_getcpu() << ") Packet of " << current_size << " put in frame queue.";
                    req->length_total -= current_size;
                    req->length_got = 0;
                    req->length_requested = sizeof(Frame_hdr);
//						LOG_DEBUG << "(" << sched_getcpu() << ") Get another packet #" << req->reassembled_frames_list.size() << " from fd #" << frame->fd << " frome same frame.";

                    req->reassembled_frames_list.push_back(std::allocate_shared<Message,TrillekAllocator<Message>>
                                                                (TrillekAllocator<Message>(),req->fd, req->CxData()));
                    // reset the timestamp to now
                    req->UpdateTimestamp();
                    // requeue the frame request for next message
                    temp_input.push_back(std::move(req));
                    ret = REPEAT;
                }
                else {
                    // request completed
                    // check integrity
                    req->CheckIntegrityTag<checkAuth>();
                    // We unlock the socket and allow again events on this socket
                    req->CxData()->ReleaseConnection();

//						LOG_DEBUG << "(" << sched_getcpu() << ") Moving " << req->reassembled_frames_list.size() << " messages with a total of " << req->length_got << " bytes to queue";
                    // Put the messages in the queue for next step
                    temp_output.splice(temp_output.end(), std::move(req->reassembled_frames_list));
                    if(ret == REPEAT) {
                        // there are still uncomplete frames, so we requeue the current block
                        ret = SPLIT;
                    }
                }
            }
        }
        // we reinject the undone work for the next job
        input->PushList(std::move(temp_input));
        // we push the result of the current job
        output->PushList(std::move(temp_output));
        return ret;
    }
};
} // network
} // trillek

#endif // NETWORKCONTROLLER_H_INCLUDED
