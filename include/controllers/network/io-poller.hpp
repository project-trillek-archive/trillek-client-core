#ifndef IOPOLLER_H_INCLUDED
#define IOPOLLER_H_INCLUDED

#include <vector>
#include <cstddef>
#include <cstdio>
#include "sys/event.h"
#include "sys/time.h"
#include <iostream>

namespace trillek { namespace network {

/** \brief C++ implementation of libkqueue
 */
class IOPoller {
public:
    IOPoller() {};
    virtual ~IOPoller() {};

    /** \brief Initialize kqueue
     *
     * \return bool true if no error was triggered
     *
     */
    bool Initialize() const {
        std::call_once(only_one,[&]() {
            kqhandle = kqueue();
        });
        return kqhandle != -1;
    }

    // copy functions are deleted
    IOPoller(IOPoller&) = delete;
    IOPoller& operator=(IOPoller&) = delete;

    /** \brief Add a socket to watch.
     *
     * The kevent structure is initialized with EV_DISPATCH, meaning that
     * the socket must be activated again with Watch() after each triggered
     * event.
     *
     * \param fd const int the file descriptor to watch
     *
     */
    void Create(const int fd) const;

   /** \brief Add a socket to watch and attach a user pointer.
     *
     * The kevent structure is initialized with EV_DISPATCH, meaning that
     * the socket must be activated again with Watch() after each triggered
     * event.
     *
     * The pointer provided will be returned in the kevent struct.
     *
     * \param fd const int the file descriptor to watch
     * \param udata void* the pointer to attach to the kevent structure
     *
     */
    void Create(const int fd, void* udata) const;

    /** \brief Add a socket to watch without EV_DISPATCH flag.
     *
     * \param fd const int the file descriptor
     *
     */
    void CreatePermanent(int fd) const;

    /** \brief Enable an event.
     *
     * Must be called after Poll().
     *
     * \param fd const int the file descriptor
     *
     */
    void Watch(const int fd) const;

    /** \brief Disable an event
     *
     * \param fd const int the file descriptor
     *
     */
    void Unwatch(const int fd) const;

    /** \brief Remove a socket to watch
     *
     * \param fd const int the file descriptor
     *
     */
    void Delete(const int fd) const;

    /** \brief Extract the list of event
     *
     * \param v std::vector<struct kevent>& the vector to fill
     * \return int the number of events
     *
     */
    int Poll(std::vector<struct kevent>& v) const;
private:
    class IOEvent;			// An IO event
    mutable int kqhandle;			// the handle of the kqueue
    const timespec ts{};	// for non-blocking kevent
    mutable std::once_flag only_one;
};

// implementation of the functions are here for inline

class IOPoller::IOEvent {
public:
    IOEvent(int fd, int filter, int operation, void* udata = NULL) {
        EV_SET(&ke, fd, filter, operation, 0, 5, udata);
    };

    virtual ~IOEvent() {};

    const struct kevent* getStruct() const { return &ke; };

private:
    struct kevent ke;
};

inline void IOPoller::Create(int fd) const {
    IOEvent e(fd, EVFILT_READ, EV_ADD|EV_DISPATCH, NULL);
    auto i = kevent(kqhandle, e.getStruct(), 1, NULL, 0, NULL);
    if (i == -1) {
        perror ("The following error occurred in Create(): ");
        return;
    }
}

inline void IOPoller::Create(int fd, void* udata) const {
    IOEvent e(fd, EVFILT_READ, EV_ADD|EV_DISPATCH, udata);
    kevent(kqhandle, e.getStruct(), 1, NULL, 0, NULL);
}

inline void IOPoller::CreatePermanent(int fd) const {
    IOEvent e(fd, EVFILT_READ, EV_ADD);
    auto i = kevent(kqhandle, e.getStruct(), 1, NULL, 0, NULL);
    if (i == -1) {
        perror ("The following error occurred in CreatePermanent(): ");
        return;
    }
}

inline void IOPoller::Watch(const int fd) const {
    IOEvent e(fd, EVFILT_READ, EV_ENABLE);
    auto i = kevent(kqhandle, e.getStruct(), 1, NULL, 0, NULL);
    if (i == -1) {
        perror ("The following error occurred in Watch(): ");
        return;
    }
}

inline void IOPoller::Unwatch(int fd) const {
    IOEvent e(fd, EVFILT_READ, EV_DISABLE);
    auto i = kevent(kqhandle, e.getStruct(), 1, NULL, 0, NULL);
}

inline void IOPoller::Delete(int fd) const {
    IOEvent e(fd, EVFILT_READ, EV_DELETE);
    auto i = kevent(kqhandle, e.getStruct(), 1, NULL, 0, NULL);
}

inline int IOPoller::Poll(std::vector<struct kevent>& v) const {
    return kevent(kqhandle, NULL, 0, v.data(), v.size(), &ts);
}
} // network
} // trillek

#endif // IOPOLLER_H_INCLUDED
