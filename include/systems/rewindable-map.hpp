#ifndef REWINDABLE_MAP_HPP_INCLUDED
#define REWINDABLE_MAP_HPP_INCLUDED
#include <iostream>
#include "bitmap.hpp"
#include "trillek-allocator.hpp"
#include "systems/async-data.hpp"
#include "transform.hpp"

namespace trillek {

template<class L,class W>
using SharedContainerConst = std::map<L,const W, std::less<L>,TrillekAllocator<std::pair<const L,W>>>;

template<class L,class W>
using SharedContainer = std::map<L,W, std::less<L>,TrillekAllocator<std::pair<const L,W>>>;

template<class L,class W>
using History = std::pair<HistoryMap<SharedContainerConst<L,W>>,HistoryMap<SharedContainerConst<L,W>>>;


/** \brief Navigable map that mimick the behaviour of a git repository
 *
 * IMPORTANT: Only 'const' functions are thread-safe and support concurrent access.
 *
 * K and V are the key and the value types of the map.
 * Timepoint is the type used for comparison of order of commits
 * HistorySize is the capacity of the history of commits.
 */
template<class K, class V, class Timepoint, int HistorySize>
class RewindableMap {

    typedef TrillekAllocator<std::pair<const K,V>> allocator_type;
    typedef TrillekAllocator<std::pair<const K,const V>> const_allocator_type;
public:
    /** \brief Default constructor
     *
     */
    RewindableMap() : rewinded(false), highest_timepoint(-1),
                    head_timepoint(-1) {};

    /** \brief Insert a new pair in the workspace map.
     *
     * The pair is not inserted if the current HEAD is not the top of the commit stack or if
     * an element with this pair already exists.
     *
     * Commit() must be called to actually record the history
     *
     * \param key K the key
     * \param value V the value
     *
     */
    void Insert(K key, V value) {
        if (rewinded) {
            LOGMSGC(ERROR) << "In rewindable map: attempt to insert an element when rewinded";
            return;
        }
        auto rkey = key;
        auto rvalue = value;
        bitmap[key] = true;
        update_bitmap[key] = true;
        datas.insert(std::make_pair<const K, V>(std::move(rkey), std::move(rvalue)));
        updated.insert(std::make_pair<const K, const V>(std::move(key), std::move(value)));
    }

    /** \brief Update the value of an element in the workspace map.
     *
     * In fact the pair is removed and inserted. The current HEAD must be the top of the stack.
     *
     * Commit() must be called to actually record the history
     *
     * \param key K the key
     * \param value V the value
     *
     */
    void Update(K key, V value) {
        Remove(key);
        Insert(std::move(key), std::move(value));
    }

    /** \brief Remove an element in the workspace map.
     *
     * The current HEAD must be the top of the stack.
     *
     * Commit() must be called to actually record the history
     *
     * \param key const K& the key
     *
     */
    void Remove(const K& key) {
        if (rewinded) {
            LOGMSGC(ERROR) << "In rewindable map: attempt to remove an element when rewinded";
            return;
        }
        auto rkey = key;
        removed.insert(std::make_pair<const K, const V>(std::move(rkey), std::move(datas.at(key))));
        datas.erase(key);
        removed_bitmap[key] = true;
        bitmap[key] = false;
    }

    /** \brief Commit the modifications of the workspace map.
     *
     * It actually records the history and clean the Index.
     *
     * If the value returned is not equal to the parameter, an error occurred.
     *
     * \param tp Timepoint the time to register for this commit
     * \return Timepoint the current HEAD
     *
     */
    Timepoint Commit(Timepoint tp) {
        if (rewinded) {
            LOGMSGC(ERROR) << "In rewindable map: attempt to commit when rewinded";
            return head_timepoint;
        }
        backward_data.Publish(std::move(removed), tp);
        forward_data.Publish(std::move(updated), tp);
        backward_bitmap.Publish(std::move(removed_bitmap), tp);
        forward_bitmap.Publish(std::move(update_bitmap), tp);
        updated.clear();
        removed.clear();
        update_bitmap = BitMap<uint32_t>();
        removed_bitmap = BitMap<uint32_t>();
        highest_timepoint = std::move(tp);
        head_timepoint = highest_timepoint;
        return head_timepoint;
    }

    /** \brief Checkout a timepoint.
     *
     * Reset the workspace map to to its stata at timepoint.
     *
     * \param tp Timepoint the timepoint requested
     * \return Timepoint the timepoint actually obtained
     *
     */
    Timepoint Checkout(Timepoint tp) {
        if (tp > highest_timepoint) {
            LOGMSGC(ERROR) << "In rewindable map: attempt to checkout an inexisting commit";
            return head_timepoint;
        }
        if (tp < head_timepoint) {
            Rewind(tp);
        }
        else {
            if (rewinded) {
                Forward(tp);
            }
        }
        return head_timepoint;
    }

    /** \brief Import the history into the map.
     *
     * This works as a rebase on a git branch. This includes a checkout.
     *
     * T is type HistoryMap<const_data_type>
     *
     * \param removals T&& the removal set
     * \param additions T&& the addition set
     * \return Timepoint the current head after the command
     *
     */
    template<class T>
    Timepoint Push(T&& removals, T&& additions) {
        if (! rewinded) {
            Rebase(std::forward<T>(removals), std::forward<T>(additions));
        }
        else {
            LOGMSGC(ERROR) << "In rewindable map: attempt to push when rewinded";
        }
        return head_timepoint;
    }

    /** \brief Get the last addition data set.
     *
     * It is not to be called from another thread.
     *
     * \return const const_data_type& the addition set
     *
     */
    const SharedContainerConst<K,V>& GetLastPositiveCommit() {
        return forward_data.GetHead();
    }

    /** \brief Get the last removal data set.
     *
     * It is not to be called from another thread.
     *
     * \return const const_data_type& the removal set
     *
     */
    const SharedContainerConst<K,V>& GetLastNegativeCommit() {
        return backward_data.GetHead();
    }

    const BitMap<uint32_t>& GetLastPositiveBitMap() {
        return forward_bitmap.GetHead();
    }

    const BitMap<uint32_t>& GetLastNegativeBitMap() {
        return backward_bitmap.GetHead();
    }

    /** \brief Get the workspace map
     *
     * \return const data_type& the map
     *
     */
    const SharedContainer<K,V>& Map() {
        return datas;
    }

    const BitMap<uint32_t>& Bitmap() {
        return bitmap;
    }

    /** \brief Pull the most recent history since last visit. Thread-safe.
     *
     * last_received is set to frame_requested if no error occurred.
     * frame_requested must be the timepoint of the last frame to be retrieved,
     * i.e the current frame of the calling thread.
     *
     * The history returned has bounds ]min(last_received,rebase),requested_frame], i.e it
     * never contains last received or rebase (the returned value of rebase).
     *
     * An empty pointer rebase must be provided. If a rebase occurred, it will contain the
     * timepoint of the first frame modified. Caller must assumme that all frames since this
     * points have been modified. They are included in the history.
     *
     * Only rebase affecting data before last_received is notified. It is provided
     * as a simple way to notify backward modifications that the caller may want to know.
     *
     * The returned object can be injected in another map using Push.
     *
     * \param frame_requested const Timepoint& the last frame to return
     * \param last_received Timepoint& last visit the before-first frame to retrieve
     * \param rebase std::shared_ptr<Timepoint>& empty. Will be set to rebase timepoint.
     * \return std::pair<HistoryMap<const_data_type>,HistoryMap<const_data_type>>
     *
     */
    History<K,V> Pull
            (const Timepoint& frame_requested, Timepoint& last_received, std::shared_ptr<Timepoint>& rebase) const {
        if (last_received > highest_timepoint) {
            LOGMSGC(ERROR) << "Consumer claiming having received more frames than published";
            last_received = highest_timepoint;
            rebase.reset();
        }
        else {
            auto selected_rebase = forward_data.RebasePoint(frame_requested, last_received);
            if (selected_rebase) {
                last_received = selected_rebase->second;
                rebase = std::make_shared<Timepoint>(selected_rebase->second);
            }

            else {
                rebase.reset();
            }
        }
        auto el1 = backward_data.GetHistoryData(frame_requested, last_received);
        return { std::move(el1), forward_data.PopSync(frame_requested, last_received) };
    }

    /** \brief Pull the most recent history since last visit. Thread-safe.
     *
     * last_received is set to frame_requested if no error occurred.
     * frame_requested must be the timepoint of the last frame to be retrieved,
     * i.e the current frame of the calling thread.
     *
     * The history returned has bounds ]min(last_received,rebase),requested_frame], i.e it
     * never contains last received or rebase.
     *
     * The returned object can be injected in another map using Push.
     *
     * \param frame_requested const Timepoint& the last frame to return
     * \param last_received Timepoint& last visit the before-first frame to retrieve
     * \return std::pair<HistoryMap<const_data_type>,HistoryMap<const_data_type>>
     *
     */
    History<K,V> Pull(const Timepoint& frame_requested, Timepoint& last_received) const {
        auto el1 = backward_data.GetHistoryData(frame_requested, last_received);
        return { std::move(el1), forward_data.PopSync(frame_requested, last_received) };
    }

private:
    /** \brief Make the workspace map go backward in history
     *
     * \param tp const Timepoint& the timepoint where to go
     *
     */
    void Rewind(const Timepoint& tp) {
        auto hist_erase = forward_data.GetReverseHistoryData(tp, head_timepoint);
        auto hist_add = backward_data.GetReverseHistoryData(tp, head_timepoint);
        auto itmap_erase = hist_erase.crbegin();
        auto itmap_add = hist_add.crbegin();
        if (itmap_erase != hist_erase.crend()) {
            rewinded = true;
            for (; itmap_erase != hist_erase.crend(); ++itmap_erase, ++itmap_add) {
                for (auto itdata = itmap_erase->second.cbegin(); itdata != itmap_erase->second.cend(); ++itdata) {
                    datas.erase(itdata->first);
                }
                for (auto itdata2 = itmap_add->second.cbegin(); itdata2 != itmap_add->second.cend(); ++itdata2) {
                    datas[itdata2->first] = itdata2->second;
                }
            }
            head_timepoint = tp;
        }
    }

    /** \brief Make the workspace map go forward in history
     *
     * \param tp const Timepoint& the timepoint where to go
     *
     */
    void Forward(const Timepoint& tp) {
        Forward(backward_data.GetHistoryData(tp, head_timepoint), forward_data.GetHistoryData(tp, head_timepoint));
    }

    /** \brief Make the workspace map go forward in history
     *
     * \param removals const HistoryMap<const_data_type>& the negative maps, i.e data to remove
     * \param additions const HistoryMap<const_data_type>& the positive maps, i.e data to add
     *
     */
    void Forward(const HistoryMap<SharedContainerConst<K,V>>& removals, const HistoryMap<SharedContainerConst<K,V>>& additions) {
        auto rem_it = removals.cbegin();
        auto add_it = additions.cbegin();
        auto rem_end = removals.cend();
        auto add_end = additions.cend();
        if (rem_it == rem_end || add_it == add_end || rem_it->first != add_it->first || rem_end->first != add_end->first) {
            return;
        }
        for (; add_it != add_end; ++add_it, ++rem_it) {
            for (auto itdata_erase = rem_it->second.cbegin(); itdata_erase != rem_it->second.cend(); ++itdata_erase) {
                datas.erase(itdata_erase->first);
            }
            for (auto itdata = add_it->second.cbegin(); itdata != add_it->second.cend(); ++itdata) {
                datas[itdata->first] = itdata->second;
            }
        }
        head_timepoint = (--add_it)->first;
        if (head_timepoint == highest_timepoint) {
            rewinded = false;
        }
    }

    /** \brief Modify the history using history objects
     *
     * T is type HistoryMap<const_data_type>
     *
     * \param removals T&& the negative maps, i.e data to remove
     * \param additions T&& the positive maps, i.e data to add
     *
     */
    template<class T=HistoryMap<SharedContainerConst<K,V>>>
    void Rebase(T&& removals, T&& additions) {
        auto rem_it = removals.cbegin();
        auto add_it = additions.cbegin();
        auto rem_end = removals.cend();
        auto add_end = additions.cend();
        if (rem_it == rem_end || rem_it->first != add_it->first || rem_end->first != add_end->first) {
            return;
        }
        if (rem_it->first <= head_timepoint) {
            Checkout(rem_it->first - 1);
        }
        backward_data.Rebase(std::move(removals));
        auto next_head = forward_data.Rebase(std::move(additions));
        if (next_head > highest_timepoint) {
            highest_timepoint = next_head;
        }
        rewinded = true;
        Checkout(highest_timepoint);
    }

    // the data
    SharedContainer<K,V> datas;
    // the bitmap of the data
    BitMap<uint32_t> bitmap;
    // modifications in index
    SharedContainerConst<K,V> updated;
    SharedContainerConst<K,V> removed;
    BitMap<uint32_t> update_bitmap;
    BitMap<uint32_t> removed_bitmap;
    // the highest timepoint we can checkout (max HEAD)
    Timepoint highest_timepoint;
    // the current timepoint (HEAD)
    Timepoint head_timepoint;
    // tells if we are rewinded, i.e head_timepoint < highest_timepoint
    bool rewinded;
    // updates go here
    AsyncFrameData<SharedContainerConst<K,V>,HistorySize> forward_data;
    // old data go here
    AsyncFrameData<SharedContainerConst<K,V>,HistorySize> backward_data;
    // Bitmaps
    AsyncFrameData<BitMap<uint32_t>> forward_bitmap;
    AsyncFrameData<BitMap<uint32_t>> backward_bitmap;
};
} // namespace trillek

#endif // REWINDABLE_MAP_HPP_INCLUDED
