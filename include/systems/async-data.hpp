#ifndef ASYNCDATA_HPP_INCLUDED
#define ASYNCDATA_HPP_INCLUDED
#include <mutex>
#include <iterator>
#include <map>
#include "trillek-scheduler.hpp"
#include "logging.hpp"

namespace trillek {

/** \brief An history object
 */
template<class T>
class HistoryMap {
public:
    typedef typename std::map<frame_tp,T>::const_iterator iter_type;

    HistoryMap(typename std::map<frame_tp,T>::const_iterator begin,
                typename std::map<frame_tp,T>::const_iterator end)
        : start(std::move(begin)), stop(std::move(end))
    {};

    HistoryMap(const std::map<frame_tp,T>& importations)
        : start(importations.cbegin()), stop(importations.cend()) {}

    typename std::map<frame_tp,T>::const_iterator cbegin() const {
        return start;
    }

    typename std::map<frame_tp,T>::const_iterator cend() const {
        return stop;
    }

private:

    typename std::map<frame_tp,T>::const_iterator start;
    typename std::map<frame_tp,T>::const_iterator stop;
};

/** \brief A reverse history object
 */
template<class T>
class ReverseHistoryMap {
public:
    typedef typename std::reverse_iterator<typename HistoryMap<T>::iter_type> iter_type;

    ReverseHistoryMap(typename std::map<frame_tp,T>::const_reverse_iterator last,
                typename std::map<frame_tp,T>::const_reverse_iterator first)
        : start(std::move(last)), stop(std::move(first))
        {};

    typename std::map<frame_tp,T>::const_reverse_iterator crbegin() const {
        return start;
    }

    typename std::map<frame_tp,T>::const_reverse_iterator crend() const {
        return stop;
    }

private:
    typename std::map<frame_tp,T>::const_reverse_iterator start;
    typename std::map<frame_tp,T>::const_reverse_iterator stop;
};

/** \brief A data with navigable history
 *
 * T is the data type
 * Historysize is the size of the history
 */
template<class T,int HistorySize = 30>
class AsyncFrameData {
    typedef std::map<frame_tp,T> content_map;
public:
    AsyncFrameData() : current_frame(-1) {
        for (auto i = -HistorySize-1000000000; i < -1000000000; ++i) {
            frame_tp a{i};
            rebase_timepoint.emplace_hint(rebase_timepoint.end(), std::pair<frame_tp,frame_tp>(a, a));
            datas.emplace_hint(datas.end(), std::make_pair<frame_tp,T>(std::move(a), T()));
        }
    };

    /** \brief Return the frames between last_received and frame_requested
     *
     * last_received is not included, frame_requested may be included.
     *
     * last_received will be updated if some data is retrieved
     *
     * Blocks if the call is before the publication of frame_requested,
     * and returns no data after 0.5s
     *
     * \param frame_requested const frame_tp& the "now" of the caller
     * \param last_received frame_tp& the last frame received by the caller
     * \return HistoryMap An object that with begin(), end() and size() functions
     *
     */
    HistoryMap<T> PopSync(const frame_tp& frame_requested, frame_tp& last_received) const {
        std::unique_lock<std::mutex> locker(m_current);
        auto stop = datas.upper_bound(frame_requested);
        if (datas.lower_bound(frame_requested) == datas.cend()) {
            if ( ! ahead_request.wait_for(locker, std::chrono::milliseconds(500), [&](){ return datas.lower_bound(frame_requested) != datas.cend(); })) {
                // 500 ms have passed, let return a empty object
                LOGMSGC(WARNING) << "PopSync: Seems that we are ahead of the publisher at frame " << frame_requested;
                return HistoryMap<T>(datas.cend(), datas.cend());
            }
            stop = datas.upper_bound(frame_requested);
        }
        auto start_index = datas.upper_bound(last_received);
        if (stop != datas.cend() && (start_index == datas.cend() || start_index->first >= stop->first)) {
            return HistoryMap<T>(datas.cend(), datas.cend());
        }
        if (start_index != stop) {
            auto it = stop; // make a copy
            last_received = (--it)->first;
        }
        return HistoryMap<T>{std::move(start_index), std::move(stop)};
    };

    /** \brief Return the frames between last_received and frame_requested
     *
     * last_received is not included, frame_requested may be included.
     *
     * last_received will be updated if some data is retrieved
     *
     * Rebase will be updated with the rebase point, if data received before last_received
     * has been modified
     *
     * Blocks if the call is before the publication of frame_requested,
     * and returns no data after 0.5s
     *
     * \param frame_requested const frame_tp& the "now" of the caller
     * \param last_received frame_tp& the last frame received by the caller
     * \param rebase std::shared_ptr<frame_tp>& the rebase timepoint or empty
     * \return HistoryMap An object that with begin(), end() and size() functions
     *
     */
    HistoryMap<T> PopSync(const frame_tp& frame_requested, frame_tp& last_received, std::shared_ptr<frame_tp>& rebase) const {
        auto selected_rebase = RebasePoint(frame_requested, last_received);
        if (selected_rebase) {
            last_received = selected_rebase->second;
            rebase = std::make_shared<frame_tp>(std::move(selected_rebase->second));
        }
        else {
            rebase.reset();
        }
        return PopSync(frame_requested, last_received);
    };

    /** \brief Return the frames between last_received and frame_requested
     *
     * last_received is included, frame_requested is not included.
     *
     * Blocks if the call is before the publication of new data since last_received,
     * and returns no data after 0.01s
     *
     * \param frame_requested const frame_tp& the "now" of the caller
     * \param last_received frame_tp& the last frame received by the caller
     * \return HistoryMap An object that with begin(), end() and size() functions
     *
     */
    HistoryMap<T> GetHistoryData(const frame_tp& frame_requested, const frame_tp& last_received) const {
        std::unique_lock<std::mutex> locker(m_current);
        auto stop = datas.upper_bound(frame_requested);
        if (datas.lower_bound(frame_requested) == datas.cend()) {
            if ( ! ahead_request.wait_for(locker, std::chrono::milliseconds(500), [&](){ return datas.lower_bound(frame_requested) != datas.cend(); })) {
                // 500 ms have passed, let return a empty object
                LOGMSGC(WARNING) << "GetHistoryData: Seems that we are ahead of the publisher at frame " << frame_requested;
                return HistoryMap<T>(datas.upper_bound(last_received), datas.cend());
            }
            stop = datas.upper_bound(frame_requested);
        }
        auto start_index = datas.upper_bound(last_received);
        if (stop != datas.cend() && (start_index == datas.cend() || start_index->first >= stop->first)) {
                return HistoryMap<T>(datas.cend(), datas.cend());
        }
        return HistoryMap<T>{std::move(start_index), std::move(stop)};
    };

    /** \brief Return the frames between first and last frame in reverse order
     *
     * first is never included
     * last is always included
     *
     * The object returned give reverse iterators from last to first.
     *
     * This allows to return in a previous state.
     *
     * Blocks if the call is before last_frame, and returns no data after 0.01s
     *
     * \param first_frame const frame_tp& the first frame
     * \param last_frame const frame_tp& the last frame
     * \return ReverseHistoryMap<T> the iterable frames
     *
     */
    ReverseHistoryMap<T> GetReverseHistoryData(const frame_tp& first_frame, const frame_tp& last_frame) const {
        std::unique_lock<std::mutex> locker2(m_current2);
        auto start_index = datas.upper_bound(last_frame);
        if (start_index == datas.cend() || last_frame > current_frame) {
            // we block until the frame time or after 10 ms
            if (! ahead_request2.wait_for(locker2, std::chrono::milliseconds(500), [&](){ return last_frame <= current_frame; })) {
                // 500 ms have passed, let return a empty object
                LOGMSGC(WARNING) << "GetReverseHistoryData: Seems that we are ahead of the publisher at " << last_frame;
                return ReverseHistoryMap<T>(datas.crend(), datas.crend());
            }
            start_index = datas.upper_bound(last_frame);
        }
        return ReverseHistoryMap<T>(typename ReverseHistoryMap<T>::iter_type(std::move(start_index)), typename ReverseHistoryMap<T>::iter_type(datas.upper_bound(first_frame)));
    };

    /** \brief Return the data in history
     *
     * \param frame const frame_tp& the timepoint to get
     * \return T the data
     *
     */
    T GetCommit(const frame_tp& frame) const {
        std::unique_lock<std::mutex> locker(m_current);
        std::unique_lock<std::mutex> locker2(m_current2);
        if (datas.count(frame)) {
            return datas.at(frame);
        }
        LOGMSGC(ERROR) << "GetCommit(): The requested commit does not exist";
        return T();
    }

    /** \brief Return the rebase point, if any
     *
     * A rebase point is a timepoint where data has been modified at some time after last_received,
     * but before the current timepoint, i.e the caller has outdated data and does not yet know it.
     *
     * The pair returned is the timepoint of the rebase operation, and the first position that was updated.
     *
     * \param tp const frame_tp& the current timepoint
     * \param last_received const frame_tp& the last timepoint received by the caller
     * \return std::shared_ptr<std::pair<const frame_tp,frame_tp>> the timepoint, or empty
     *
     */
    std::shared_ptr<std::pair<const frame_tp,frame_tp>> RebasePoint(const frame_tp& tp, const frame_tp& last_received) const {
        std::unique_lock<std::mutex> locker(rebase_m);
        auto rebase = rebase_timepoint.upper_bound(last_received);
        if (rebase != rebase_timepoint.cend() && rebase->second < last_received && rebase->first <= tp) {
                return std::make_shared<std::pair<const frame_tp,frame_tp>>(std::move(*rebase));
        }
        return std::shared_ptr<std::pair<const frame_tp,frame_tp>>();

    }

    /** \brief Make the data available to all threads
     *
     * The threads waiting a publication are unblocked
     *
     * \param data const T& the data to broadcast
     *
     */
    template<class U=const T>
    void Publish(U&& data, frame_tp frame) {
        {
            std::unique_lock<std::mutex> locker(m_current);
            std::unique_lock<std::mutex> locker2(m_current2);
            current_frame = std::move(frame);
            datas[current_frame] = std::forward<U>(data);
            datas.erase(datas.begin());
        }
        ahead_request.notify_all();
        ahead_request2.notify_all();
    };

    /** \brief Get the data of the last frame available - Not thread-safe
     *
     * \return const T& the data
     *
     */
    const T& GetHead() {
        return datas.at(current_frame);
    }

    /** \brief Update the data using an history object and checkout the most recent commit
     *
     * \param commits HistoryMap<T>&& the history object
     * \return frame_tp the timepoint currently checked out
     *
     */
    frame_tp Rebase(HistoryMap<T>&& commits) {
        auto commit = commits.cbegin();
        auto it_end = commits.cend();
        if (commit == it_end) {
            return current_frame;
        }
        auto next_highest = std::max((--commits.cend())->first, current_frame);
        if (commit->first <= current_frame) {
            // the 1st commit is before the max head
            // skip if a rebase point is ahead since we don't update backward
            for (auto it = rebase_timepoint.lower_bound(commit->first); it != rebase_timepoint.cend(); ++it) {
                if (it->second > commit->first) {
                    return current_frame;
                }
            }
            // if we are ahead of the last rebase point but before the current head, update the rebase point
            std::unique_lock<std::mutex> locker(rebase_m);
            rebase_timepoint[next_highest] = commit->first - 1;
            rebase_timepoint.erase(rebase_timepoint.cbegin());
        }
        for (; commit != it_end; ++commit) {
            // replace the original or publish it
            Amend(std::move(commit->second), std::move(commit->first));
        }
        current_frame = next_highest;
        return current_frame;
    }

private:
    /** \brief Modify the data, in history or not
     *
     * \param data U&& the data
     * \param frame const frame_tp& the timepoint to modify or to add
     *
     */
    template<class U=const T>
    void Amend(U&& data, const frame_tp& frame) {
        {
            std::unique_lock<std::mutex> locker(m_current);
            std::unique_lock<std::mutex> locker2(m_current2);
            if (frame <= current_frame) {
                datas.at(frame) = std::forward<U>(data);
                return;
            }
        }
        Publish(std::forward<U>(data), frame);
    };

    content_map datas;
    frame_tp current_frame;
    std::map<frame_tp,frame_tp> rebase_timepoint;
    mutable std::mutex rebase_m;
    mutable std::mutex m_current;
    mutable std::mutex m_current2;
    mutable std::condition_variable ahead_request;
    mutable std::condition_variable ahead_request2;
};
} // namespace trillek

#endif // ASYNCDATA_HPP_INCLUDED
