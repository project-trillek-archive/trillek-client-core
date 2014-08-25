#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <cstdint>
#include <memory>
#include <vector>
#include <cstring>
#include "controllers/network/network.hpp"
#include "controllers/network/packet-handler.hpp"

// size of the VMAc tag
#define VMAC_SIZE			8
// size of the ESIGN tag
#define ESIGN_SIZE          24
// size of the counter
#define COUNTER_SIZE        4

// WARNING : We are little endian even on network !!!

// Version MAJOR codes
// unauthenticated messages: everyone can send them
#define TEST_MSG			0		// Reserved
#define NET_MSG 		1		// low level messages for authentication and disconnect
#define SERVER_MSG		2		// Server public information
#define PUBPLAYER_MSG	3		// Player public profile
#define	DLBINARY_MSG	4		// for binary download
#define	ASSETS_MSG		5		// for public assets download
#define	RESERVED_6		6
#define	RESERVED_7		7
// authenticated messages: only authenticated users can send and receive this
#define	PROFILE_MSG		8		// player private profile
#define WORLD_MSG		9		// world data
#define GAME_MSG		10		// game data
#define SOCIAL_MSG		11		// chat/mail
#define PASSETS_MSG		11		// private assets
#define CPU_MSG			12		// for CPU...
// Feel free to add or modify this list

#define IS_RESTRICTED(x)	((x >> 3) != 0)

namespace trillek { namespace network {

class ConnectionData;

/** \brief The header of the message, without the preceding length
 */
struct msg_hdr {
    uint32_t flags;
    unsigned char type_major;
    unsigned char type_minor;
    char padding[6];
};

struct msg_tail {
    id_t entity_id;
    unsigned char tag[ESIGN_SIZE];
};

/** \brief The header of the frame, i.e the length
 */
struct Frame_hdr {
    uint32_t length;
};

/** \brief The header including length
 */
struct Frame {
    Frame_hdr fheader;
    msg_hdr mheader;
};

class Message {
public:
    friend class Authentication;
    template<int Major,int Minor, TagType T> friend void packet_handler::PacketHandler::Process() const;
    friend class NetworkController;
    friend class Frame_req;

    /** \brief Constructor
     *
     * \param T the type of packet to construct
     * \param fd int the file descriptor of the socket (optional)
     * \param cxdata_ptr const ConnectionData* const the object storing
     * data about the connection
     *
     */
    template<class U=Frame>
    Message(const int fd = -1, const ConnectionData* const cx_data = nullptr) :
            cx_data(cx_data),
            fd(fd), packet_size(sizeof(U)),
            data(std::vector<char>(sizeof(U) + sizeof(msg_tail))) {};

    virtual ~Message() {};

    // Copy constructor and assignment are deleted to remain zero-copy
    Message(Message&) = delete;
    Message& operator=(Message&) = delete;

    // Move constructor
    Message(Message&& m) :
        data(std::move(m.data)),
        packet_size (std::move(m.packet_size)),
        cx_data (std::move(m.cx_data)),
        fd(std::move(m.fd)) {};

    template<class U>
    Message& operator<<(const U& in) {
        append(&in, sizeof(U));
        return *this;
    }

    /** \brief Send a message to a client using TCP
     *
     * \param id id_t the id of the entity to which the message must be sent
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void Send(id_t id, unsigned char major, unsigned char minor);

    /** \brief Send a message to a server using TCP
     *
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void Send(unsigned char major, unsigned char minor);

    /** \brief Resize the internal buffer.
     *
     * This function is specialized for the case where there is no VMAC tag
     * to include
     *
     * \param new_size size_t the new size
     *
     */
    template<TagType tag=SERVER>
    void Resize(size_t new_size);

    /** \brief Maps the body of the message to the structure of
     * the packet of type T
     *
     * The internal buffer is dynamically resized.
     *
     * \return U* pointer to the packet T
     *
     */
    template<class V, class U=V>
    U* Content() {
        if(packet_size < sizeof(V) + sizeof(Frame)) {
            Resize(sizeof(V));
        }
        return reinterpret_cast<U*>(Body());
    }

    /** \brief Return the size of the frame including header,
     * but without VMAC tag
     *
     * \return uint32_t the size
     *
     */
    uint32_t PacketSize() const { return packet_size; };

    /** \brief Return the entity id attached to this message
     *
     * \return id_t the id
     *
     */
    id_t GetId() const;

    /** \brief Return a pointer on the header of the message
     *
     * \return msg_hdr* the pointer on msg_hdr
     *
     */
    msg_hdr* Header() {
        return reinterpret_cast<msg_hdr*>(data.data() + sizeof(Frame_hdr));
    };

    /** \brief Return a pointer on the frame, beginning at the length field
     *
     * \return Frame_hdr* the pointer
     *
     */
    Frame_hdr* FrameHeader() {
        return reinterpret_cast<Frame_hdr*>(data.data());
    };

private:
    /** \brief Remove the VMAC tag
     *
     */
    void RemoveVMACTag() { packet_size -= VMAC_SIZE; };
    void RemoveTailClient() { packet_size -= sizeof(msg_tail); };
    char* Body() {
        return (data.data() + sizeof(msg_hdr) + sizeof(Frame_hdr));
    };

    uint32_t BodySize() {
        return (Tail<unsigned char*>() - reinterpret_cast<unsigned char*>(Body()));
    };

    template<class T>
    T Tail() {
        return reinterpret_cast<T>(data.data() + packet_size);
    };

    const ConnectionData* CxData() const { return cx_data; };
    int FileDescriptor() const { return fd; };

    void Send(int fd, unsigned char major, unsigned char minor,
        const std::function<void(unsigned char*,const unsigned char*,size_t)>& hasher,
        unsigned char* tagptr,
        unsigned int tag_size,
        unsigned int tail_size);

    void SendMessageNoVMAC(int fd, unsigned char major, unsigned char minor);

    /** \brief Append data to the message using the copy method of the object
     *
     * \param data const T& the data
     *
     */
    template<class T>
    void append(const T& data) {
        Resize(BodySize() + sizeof(T));
        *reinterpret_cast<T*>(Tail<unsigned char*>() - sizeof(T)) = data;
    }

    /** \brief Append data to the message using memcpy function
     *
     * \param in const void* the pointer on the data
     * \param sizeBytes std::size_t the number of bytes to append
     *
     */
    void append(const void* in, std::size_t sizeBytes);

    std::vector<char> data;
    uint32_t packet_size;
    const ConnectionData* const cx_data;
    const int fd;
};

// Declare specialized template functions.
template<> Message& Message::operator<<(const std::string& in);
template<> Message& Message::operator<<(const std::vector<unsigned char>& in);
} // network
} // trillek

#endif // MESSAGE_H_INCLUDED
