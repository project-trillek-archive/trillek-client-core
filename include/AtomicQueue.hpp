#ifndef ATOMICQUEUE_HPP_INCLUDED
#define ATOMICQUEUE_HPP_INCLUDED

#include <mutex>
#include <atomic>
#include <list>
#include <memory>

namespace trillek {

    template<class T>
    using atomic_queue = std::list<T>;

    /** \brief A thread-safe queue implementation with atomic operations
     */
    template<class T>
    class AtomicQueue {
    public:

        /** \brief Default constructor
         *
         */
        AtomicQueue() {};

        /** \brief Destructor
         *
         */
        virtual ~AtomicQueue() { mtx.unlock(); };

        // disable copy functions
        AtomicQueue(AtomicQueue&) = delete;
        AtomicQueue& operator=(AtomicQueue&) = delete;

        /** \brief Empty the queue and return the content
         *
         * \return atomic_queue<T> A list of the content
         *
         */
        atomic_queue<T> Poll() const {
            std::unique_lock<std::mutex> locker(mtx);
            if (! q.size()) {
                return {};
            }
            auto ret = atomic_queue<T>{};
            std::swap(ret,q);
            return ret;
        }

        /** \brief Put an element at the end of the queue
         *
         * \param element U&& element to put in the queue
         */
        template<class U>
        void Push(U&& element) const {
            std::unique_lock<std::mutex> locker(mtx);
            q.push_back(std::forward<U>(element));
        }

        /** \brief Put a list of element at the end of the queue
         *
         * \param list U&& list of elements to add
         */
        template<class U>
        void PushList(U&& list) const {
            std::unique_lock<std::mutex> locker(mtx);
            q.splice(q.end(), std::forward<U>(list));
        }

        /** \brief Pop an element from the end of the queue
         *
         * \param element T& reference that will contain the element popped
         * \return bool true if an element was popped, false otherwise
         */
        bool Pop(T& element) const {
            std::unique_lock<std::mutex> locker(mtx);
            if(q.empty()) {
                return false;
            }
            element = std::move(q.front());
            q.pop_front();
            return true;
        }

        /** \brief Test if the queue is empty
         *
         * \return bool true if the queue is empty, false otherwise
         *
         */
        bool Empty() const {
            std::unique_lock<std::mutex> locker(mtx);
            return q.empty();
        }

    private:

        // the queue
        mutable atomic_queue<T> q;
        // the mutex protecting the queue
        mutable std::mutex mtx;

    };
}


#endif // ATOMICQUEUE_HPP_INCLUDED
