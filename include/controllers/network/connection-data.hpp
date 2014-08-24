#ifndef CONNECTIONDATA_H_INCLUDED
#define CONNECTIONDATA_H_INCLUDED

#include <atomic>
#include "controllers/network/network.hpp"

using network::TCPConnection;

namespace trillek { namespace network {

/** \brief This object is attached to each socket
 */
class ConnectionData {
public:
    /** \brief Constructor
     *
     * \param state const unsigned char the initial state of the client
     * \param connection the connection instance
     *
     */
    ConnectionData(const unsigned char state, TCPConnection&& connection) :
        _auth_state(state),
        _connection(std::move(connection)),
        _id(0) {};

    /** \brief Constructor
     *
     * Note that the connection is considered authenticated
     *
     * \param id const id_t the id of the entity
     * \param hasher the hasher functor to send packets
     * \param verifier the verifier functor to check packet received
     *
     */
    ConnectionData(const id_t id,
        std::function<void(unsigned char*,const unsigned char*,size_t)>&& hasher,
        std::function<bool(const unsigned char*,const unsigned char*,size_t)>&& verifier)
            : _auth_state(AUTH_SHARE_KEY), _id(id), _hasher(std::move(hasher)), _verifier(std::move(verifier)) {};

    virtual ~ConnectionData() {};

    // Copy functions are deleted
    ConnectionData(ConnectionData&) = delete;
    ConnectionData& operator=(ConnectionData&) = delete;

    /** \brief Compare atomically the current state of the connection
     *
     * \param state unsigned char the state to compare with
     * \return bool true if equal, false otherwise
     *
     */
    bool CompareAuthState(unsigned char state) const {
        return (_auth_state.load() == state);
    }

    /** \brief Set atomically the state of the connection
     *
     * The previous state is checked for allowed transitions
     * AUTH_NONE is always allowed.
     *
     * true is returned to the first thread calling the transition
     * false is returned to other threads for the same transition
     *
     * \param state unsigned char the state to set
     * \return bool true if the new state was allowed and set
     *
     */
    bool SetAuthState(unsigned char state) const {
        if (! state) {
            // AUTH_NONE
            _auth_state.store(state);
            return false;
        }
        auto previous = const_cast<unsigned char*>(&_states.at(state-1));
        return std::atomic_compare_exchange_strong(&_auth_state, previous, state);
    }

    /** \brief Return the state of the connection
     *
     * \return unsigned char the state
     *
     */
    unsigned char AuthState() const {
        return _auth_state.load();
    }

    /** \brief Try to lock the mutex associated with this socket
     *
     * \return bool true if the lock is acquired, false otherwise
     *
     */
    bool TryLockConnection() const { return _cx_mutex.try_lock(); };

    /** \brief Release the mutex associated with this connection
     *
     */
    void ReleaseConnection() const { _cx_mutex.unlock(); };

    /** \brief Return the hasher associated to this socket
     *
     * \return const std::function<void(unsigned char*,const unsigned char*,size_t)>* const
     *
     */
    const std::function<void(unsigned char*,const unsigned char*,size_t)>& Hasher() const {
        return _hasher;
    };

    /** \brief Return the verifier associated to this socket
     *
     * \return const std::function<bool(const unsigned char*,const unsigned char*,size_t)>* const
     *
     */
    const std::function<bool(const unsigned char*,const unsigned char*,size_t)>& Verifier() const {
        return _verifier;
    };

    /** \brief Return the id of the entity to which this socket is attached
     *
     * \return id_t the id
     *
     */
    id_t Id() const { return _id; };

    /** \brief Get the instance of TCPConnection
     *
     * \return TCPConnection the connexion
     *
     */
    TCPConnection ConnectionAccept() const {
        unsigned char key_exchange_state = AUTH_KEY_EXCHANGE;
        unsigned char share_key_state = AUTH_SHARE_KEY;
        if (std::atomic_compare_exchange_strong(&_auth_state, &key_exchange_state, share_key_state)) {
            // single thread
            return std::move(_connection);
        }
        return {};
    };

private:
    // connection write is protected by _auth_state single-threaded transition. no read
    mutable TCPConnection _connection;
    mutable std::atomic<unsigned char> _auth_state;
    static const std::vector<unsigned char> _states;
    const id_t _id;
    const std::function<void(unsigned char*,const unsigned char*,size_t)> _hasher;
    const std::function<bool(const unsigned char*,const unsigned char*,size_t)> _verifier;
    // guards the access to the socket
    mutable std::mutex _cx_mutex;
};
} // network
} // trillek
#endif // CONNECTIONDATA_H_INCLUDED
