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
     * \return std::shared_future<<std::shared_ptr<const T>>> the future
     *
     */
    std::shared_future<std::shared_ptr<const T>> GetFuture(const frame_tp& frame_requested) const {
        std::unique_lock<std::mutex> locker(m_current);
        return (frame_requested <= current_frame) ? current_future : std::shared_future<std::shared_ptr<const T>>();
    };

    /** \brief Make the data available to all threads
     *
     * The futures are made ready
     *
     * \param data const T the data to broadcast
     * \param frame const frame_tp& the current frame
     *
     */
    template<class U=std::shared_ptr<const T>>
    void Publish(U&& data, frame_tp frame) {
        // unblock threads waiting the data
        current_promise.set_value(std::forward<U>(data));
        // update the frame timepoint
        std::unique_lock<std::mutex> locker(m_current);
        current_frame = std::move(frame);
    };

    /** \brief Remove access to current data
     *
     * After the call, the futures distributed will be ready only at the next Publish() call
     *
     */
    void Unpublish() {
        std::unique_lock<std::mutex> locker(m_current);
        // set the promise
        current_promise = std::promise<std::shared_ptr<const T>>();
        current_future = current_promise.get_future().share();
    }

private:
    std::promise<std::shared_ptr<const T>> current_promise;
    std::shared_future<std::shared_ptr<const T>> current_future;
    frame_tp current_frame;
    mutable std::mutex m_current;
};
} // namespace trillek

#endif // ASYNCDATA_HPP_INCLUDED
