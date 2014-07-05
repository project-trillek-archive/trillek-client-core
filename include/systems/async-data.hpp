#ifndef ASYNCDATA_HPP_INCLUDED
#define ASYNCDATA_HPP_INCLUDED

#include <future>
#include <mutex>
#include "trillek-scheduler.hpp"

namespace trillek {

template<class T>
class AsyncData {
public:
    AsyncData() {
        Unpublish();
    };

    /** \brief Request a future for the data
     *
     * The future returned is not valid if the frame requested does not match
     * the current frame relative to the publisher
     *
     * Callers must catch exceptions thrown through the future.
     *
     * \param frame_requested const frame_tp& the current frame of the caller
     * \return std::shared_future<constT> the future
     *
     */
    std::shared_future<const T> GetFuture(const frame_tp& frame_requested) const {
        std::unique_lock<std::mutex> locker(m_current);
        return (frame_requested <= current_frame) ? current_future : std::shared_future<const T>();
    };

    /** \brief Make the data available to all threads
     *
     * The futures are made ready
     *
     * \param data const T the data to broadcast
     * \param frame const frame_tp& the current frame
     *
     */
    void Publish(const T& data, const frame_tp& frame) const {
        std::unique_lock<std::mutex> locker(m_current);
        current_frame = frame;
        current_promise.set_value(data);
    };

    /** \brief Remove access to current data
     *
     * The futures distributed will be ready at the next Publish() call
     *
     */
    void Unpublish() const {
        std::unique_lock<std::mutex> locker(m_current);
        // set the promise
        current_promise = std::promise<const T>();
        current_future = current_promise.get_future().share();
    }

private:
    mutable std::promise<const T> current_promise;
    mutable std::shared_future<const T> current_future;
    mutable frame_tp current_frame;
    mutable std::mutex m_current;
};
} // namespace trillek

#endif // ASYNCDATA_HPP_INCLUDED
