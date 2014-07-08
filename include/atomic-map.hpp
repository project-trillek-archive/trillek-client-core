#ifndef ATOMICMAP_HPP_INCLUDED
#define ATOMICMAP_HPP_INCLUDED

#include <mutex>
#include <atomic>
#include <map>

namespace trillek {

/** \brief A thread-safe map implementation with atomic operations
 */
template<class K,class T>
class AtomicMap {

public:

    /** \brief Default constructor
     *
     */
    AtomicMap() {};

    /** \brief Default destructor
     *
     */
    virtual ~AtomicMap() {};

    AtomicMap(const AtomicMap<K,T>& rhs) {
        this->q = rhs.q;
    }

    AtomicMap<K,T>& operator=(const AtomicMap<K,T>& rhs) {
        this->q = rhs.q;
        return *this;
    }

    std::map<K,T> Poll() const {
        std::unique_lock<std::mutex> locker(mtx);
        if (! q.size()) {
            return {};
        }
        auto ret = std::map<K,T>{};
        std::swap(ret,q);
        return ret;
    }

    /** \brief Insert an element
     *
     * \param key K&& key of the element
     * \param value T&& value to insert
     *
     */
    template<class L=K,class U=T>
    void Insert(L&& key, U&& value) const {
        std::lock_guard<std::mutex> locker(mtx);
        q[std::forward<K>(key)] = std::forward<U>(value);
    }

    /** \brief Remove an element
     *
     * \param key const K& the key of the element to remove
     *
     */
    void Erase(const K& key) const {
        std::lock_guard<std::mutex> locker(mtx);
        q.erase(key);
    }

    /** \brief Clear the content of the map
     *
     */
    void Clear() const {
        std::lock_guard<std::mutex> locker(mtx);
        q.clear();
    }

    /** \brief Remove and get a reference of an element
     *
     * \param key const K& the key of the element
     * \param element T& a non-const reference that will contain the element
     * \return bool true if removed, false otherwise
     *
     */
    bool Pop(const K& key, T& element) const {
        std::lock_guard<std::mutex> locker(mtx);
        if(q.count(key)) {
            element = std::move(q.at(key));
            q.erase(key);
            return true;
        }
        return false;
    }

    /** \brief Get an element
     *
     * \param key const K& the key of the element
     * \return T the element
     *
     */
    T At(const K& key) const {
        std::lock_guard<std::mutex> locker(mtx);
        return q.at(key);
    }

    /** \brief Get the number of elements having key
     *
     * \param key const K& the key
     * \return size_t the number of elements
     *
     */
    size_t Count(const K& key) const {
        std::lock_guard<std::mutex> locker(mtx);
        return q.count(key);
    }

    /** \brief Compare atomically an element with a value
     *
     * \param key const K& the key of the element
     * \param element const T& the value to compare with
     * \return bool true if equal, false otherwise
     *
     */
    bool Compare(const K& key, const T& element) const {
        std::lock_guard<std::mutex> locker(mtx);
        return q.count(key) && (q.at(key) == element);
    }

private:
    // The map
    mutable std::map<K,T> q;
    // The mutex protectiong the map
    mutable std::mutex mtx;

};
}

#endif // ATOMICMAP_HPP_INCLUDED
