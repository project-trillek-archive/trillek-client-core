#ifndef FRAMEREQUEST_H_INCLUDED
#define FRAMEREQUEST_H_INCLUDED

#include <chrono>
#include "trillek.hpp"
#include "controllers/network/authentication-handler.hpp"
#include "controllers/network/connection-data.hpp"

//TODO: Remove Visual Studio specific code
// when a decent std::chrono implementation will be available

#if defined(_MSC_VER)
//3 seconds in nanoseconds
#define TIMEOUT 3000000000
#else
#define TIMEOUT 3
#endif

namespace trillek { namespace network {

/** \brief This object holds the data needed to reassemble frames.
 * Instances are provided to the reassembling block for processing.
 */
class Frame_req {
public:
    /** \brief Constructor
     *
     * fd socket_t the file descriptor of the socket
     * length_total size_t number of byte to reassemble
     * cxdata_ptr const ConnectionData* const a pointer on an object storing
     * some data on the connection
     */
    Frame_req(const netport::socket_t fd, size_t length_total, const ConnectionData* const cxdata_ptr);

    // delete copy constructor and assignment for zero-copy guarantee
    Frame_req(Frame_req&) = delete;
    Frame_req& operator=(Frame_req&) = delete;

    /** \brief Verify the VMAC tag of all the frames reassembled and remove problematic frames
     *
     */
    template<TagType T>
    void CheckIntegrityTag() const;

    /** \brief Return a pointer on the object storing data on the connection
     *
     * \return const ConnectionData* const the object
     *
     */
    const ConnectionData* const CxData() const { return cx_data; };

    /** \brief Reset the timer for this request
     *
     */
    void UpdateTimestamp();

    /** \brief Tell if the request timer reached the timeout
     *
     * \return bool true if the timeout is reached, false otherwise
     *
     */
    bool HasExpired() const;

    // the list is guarded by _cx_mutex in ConnectionData
    mutable std::list<std::shared_ptr<Message>,TrillekAllocator<std::shared_ptr<Message>>> reassembled_frames_list;
    uint32_t length_requested;
    uint32_t length_got;
    size_t length_total;
    const netport::socket_t fd;
private:
#if defined(_MSC_VER)
    std::chrono::nanoseconds expiration_time;
#else
    std::chrono::steady_clock::time_point expiration_time;
#endif
    const std::chrono::nanoseconds timeout;
    const ConnectionData* const cx_data;
};
} // network
} // trillek

#endif // FRAMEREQUEST_H_INCLUDED
