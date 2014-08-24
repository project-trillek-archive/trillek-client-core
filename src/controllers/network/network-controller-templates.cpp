#include "controllers/network/network.hpp"
#include "controllers/network/network-controller.hpp"

namespace trillek { namespace network {
template<>
void NetworkController::CloseConnection<CLIENT>(const Message* frame) const {
    delete frame->CxData();
    RemoveClientConnection();
}

template<>
void NetworkController::CloseConnection<SERVER>(const Message* frame) const {
    RemoveConnection(frame->fd);
    delete frame->CxData();
}

template<>
int NetworkController::UnauthenticatedDispatch<CLIENT>() const {
    auto req_list = GetPublicReassembledFrameQueue()->Poll();
    if (req_list.empty()) {
        return STOP;
    }
    trillek_list<std::shared_ptr<Message>> temp_send_salt;
    trillek_list<std::shared_ptr<Message>> temp_key_reply;
//					LOG_DEBUG << "(" << sched_getcpu() << ") got " << req_list->size() << " PublicDispatchReq events";
    for (auto& req : req_list) {
        msg_hdr* header = req->Header();
        auto major = header->type_major;
        if (IS_RESTRICTED(major)) {
//                LOG_DEBUG << "restricted";
            CloseConnection<CLIENT>(req.get());
            break;
        }
        switch(major) {
        case NET_MSG:
            {
                auto minor = header->type_minor;
                switch(minor) {
                    case AUTH_SEND_SALT:
                        {
                            temp_send_salt.push_back(std::move(req));
                            break;
                        }
                    case AUTH_KEY_REPLY:
                        {
                            temp_key_reply.push_back(std::move(req));
                            break;
                        }
                    default:
                        {
//                                LOG_DEBUG << "(" << sched_getcpu() << ") invalid minor code, closing";
                            CloseConnection<CLIENT>(req.get());
                        }
                }
                break;
            }
        default:
            {
//                    LOG_DEBUG << "(" << sched_getcpu() << ") invalid major code in unauthenticated chain, packet of " << req->PacketSize() << " bytes, closing";
                CloseConnection<CLIENT>(req.get());
            }
        }
    }
    if(! temp_send_salt.empty()) {
        packet_handler.GetQueue<NET_MSG,AUTH_SEND_SALT, CLIENT>().PushList(std::move(temp_send_salt));
        packet_handler.Process<NET_MSG,AUTH_SEND_SALT, CLIENT>();
    }
    if(! temp_key_reply.empty()) {
        packet_handler.GetQueue<NET_MSG,AUTH_KEY_REPLY, CLIENT>().PushList(std::move(temp_key_reply));
        packet_handler.Process<NET_MSG,AUTH_KEY_REPLY, CLIENT>();
    }
    return STOP;

}

template<>
int NetworkController::UnauthenticatedDispatch<SERVER>() const {
    auto req_list = GetPublicReassembledFrameQueue()->Poll();
    if (req_list.empty()) {
        return STOP;
    }
    trillek_list<std::shared_ptr<Message>> temp_init;
    trillek_list<std::shared_ptr<Message>> temp_key_exchange;
//		LOG_DEBUG << "(" << sched_getcpu() << ") got " << req_list->size() << " PublicDispatchReq events";
    for (auto& req : req_list) {
        msg_hdr* header = req->Header();
        auto major = header->type_major;
        if (IS_RESTRICTED(major)) {
//                LOG_DEBUG << "restricted";
            CloseConnection<SERVER>(req.get());
            break;
        }
        switch(major) {
        case NET_MSG:
            {
                auto minor = header->type_minor;
                switch(minor) {
                    case AUTH_INIT:
                        {
                            temp_init.push_back(std::move(req));
                            break;
                        }
                    case AUTH_KEY_EXCHANGE:
                        {
                            temp_key_exchange.push_back(std::move(req));
                            break;
                        }
                    default:
                        {
//                                LOG_DEBUG << "(" << sched_getcpu() << ") invalid minor code, closing";
                            CloseConnection<SERVER>(req.get());
                        }
                }
                break;
            }
        default:
            {
//                    LOG_DEBUG << "(" << sched_getcpu() << ") invalid major code in unauthenticated chain, packet of " << req->PacketSize() << " bytes, closing";
                CloseConnection<SERVER>(req.get());
            }
        }
    }
    if(! temp_init.empty()) {
        packet_handler.GetQueue<NET_MSG,AUTH_INIT, SERVER>().PushList(std::move(temp_init));
        packet_handler.Process<NET_MSG,AUTH_INIT, SERVER>();
    }
    if(! temp_key_exchange.empty()) {
        packet_handler.GetQueue<NET_MSG,AUTH_KEY_EXCHANGE, SERVER>().PushList(std::move(temp_key_exchange));
        packet_handler.Process<NET_MSG,AUTH_KEY_EXCHANGE, SERVER>();
    }
    return STOP;

}

template<>
int NetworkController::AuthenticatedDispatch<SERVER>() const {
    auto req_list = NetworkController::GetAuthenticatedCheckedFrameQueue()->Poll();
    if (req_list.empty()) {
        return STOP;
    }
    trillek_list<std::shared_ptr<Message>> temp_list;
    for (auto& req : req_list) {
        msg_hdr* header = req->Header();
        auto major = header->type_major;
        switch(major) {
            // select handlers
        case TEST_MSG:
            {
                auto minor = header->type_minor;
                switch(minor) {
                    case TEST_MSG:
                        {
                            temp_list.push_back(std::move(req));
                            break;
                        }
                    default:
                        {
//                                LOG_ERROR << "(" << sched_getcpu() << ") TEST: closing";
                            CloseConnection<SERVER>(req.get());
                        }
                }
                break;
            }

        default:
            {
//                    LOG_ERROR << "(" << sched_getcpu() << ") Authenticated switch: closing";
                CloseConnection<SERVER>(req.get());
            }
        }
    }
    if (! temp_list.empty()) {
        packet_handler.GetQueue<TEST_MSG,TEST_MSG, SERVER>().PushList(std::move(temp_list));
        packet_handler.Process<TEST_MSG,TEST_MSG, SERVER>();
    }
    return STOP;
}

template<>
int NetworkController::AuthenticatedDispatch<CLIENT>() const {
    auto req_list = NetworkController::GetAuthenticatedCheckedFrameQueue()->Poll();
    if (req_list.empty()) {
        return STOP;
    }
    trillek_list<std::shared_ptr<Message>> temp_list;
//					LOG_DEBUG << "(" << sched_getcpu() << ") got " << req_list->size() << " AuthenticatedCheckedDispatchReq events";
    for (auto& req : req_list) {
        msg_hdr* header = req->Header();
        auto major = header->type_major;
        switch(major) {
            // select handlers
        case TEST_MSG:
            {
                auto minor = header->type_minor;
                switch(minor) {
                    case TEST_MSG:
                        {
                            temp_list.push_back(std::move(req));
                            break;
                        }
                    default:
                        {
//                                LOG_ERROR << "(" << sched_getcpu() << ") TEST: closing";
                            CloseConnection<CLIENT>(req.get());
                        }
                }
                break;
            }

        default:
            {
//                    LOG_ERROR << "(" << sched_getcpu() << ") Authenticated switch: closing";
                CloseConnection<CLIENT>(req.get());
            }
        }
    }
    if (! temp_list.empty()) {
        packet_handler.GetQueue<TEST_MSG,TEST_MSG, CLIENT>().PushList(std::move(temp_list));
        packet_handler.Process<TEST_MSG,TEST_MSG, CLIENT>();
    }
    return STOP;
}
} // network
} // trillek
