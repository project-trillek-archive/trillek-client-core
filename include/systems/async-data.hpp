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
        Unpublish(frame_tp{});
    };

    /** \brief Request a future for the data
     *
     * The future returned is not valid if the frame requested is behind
     * the current frame for the publisher
     *
     * Callers must catch exceptions thrown through the future.
     *
     * \param frame_requested const frame_tp& the current frame of the caller
     * \return std::shared_future<<std::shared_ptr<const T>>> the future
     *
     */
    std::shared_future<std::shared_ptr<const T>> GetFuture(const frame_tp& frame_requested) const {
        std::unique_lock<std::mutex> locker(m_current);
        if (frame_requested < current_frame) {
            // the call is too late
            return {};
        }
        // we block until the frame time or after 500 ms
        if (! ahead_request.wait_for(locker, std::chrono::milliseconds(500), [&](){ return frame_requested <= current_frame; })) {
            // 500 ms have passed, let return a invalid future
            return {};
        }
        return current_future;
    };

    /** \brief Make the data available to all threads
     *
     * The futures are made ready
     *
     * \param data const T the data to broadcast
     *
     */
    template<class U=std::shared_ptr<const T>>
    void Publish(U&& data) {
        // unblock threads waiting the data
        current_promise.set_value(std::forward<U>(data));
    };

    /** \brief Remove access to current data
     *
     * This also declares what frame we will honour next time
     *
     * \param frame const frame_tp the current frame
     */
    void Unpublish(frame_tp frame) {
        std::unique_lock<std::mutex> locker(m_current);
        // delete the future
        current_future = std::shared_future<std::shared_ptr<const T>>();
        // set the promise
        current_promise = std::promise<std::shared_ptr<const T>>();
        current_future = current_promise.get_future().share();
        // update the frame timepoint
        current_frame = std::move(frame);
        ahead_request.notify_all();
    }

private:
    std::promise<std::shared_ptr<const T>> current_promise;
    std::shared_future<std::shared_ptr<const T>> current_future;
    frame_tp current_frame;
    mutable std::mutex m_current;
    mutable std::condition_variable ahead_request;
};
} // namespace trillek

#endif // ASYNCDATA_HPP_INCLUDED
