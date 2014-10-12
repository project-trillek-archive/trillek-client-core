#ifndef MAPARRAY_HPP_INCLUDED
#define MAPARRAY_HPP_INCLUDED

#include <map>
#include <iterator>
#include <functional>
#include "trillek.hpp"

#define CONTAINER_CHUNK_SHIFT 4
#define CONTAINER_CHUNK_MASK 0xF
#define CONTAINER_CHUNK_SIZE ((1 << CONTAINER_CHUNK_SHIFT))

namespace trillek {
typedef id_t chunk_id;

template<class T>
class MapArrayIterator;

template<class T>
/** \brief The array of primitive data
 */
struct Chunk {
    T data[CONTAINER_CHUNK_SIZE];
};

template<class T>
/** \brief A template used as a container. It has the same interface and
 * behaviour as a map but stores data in memory blocks of 16 elements.
 *
 * The block chosen to store data has a key equal to the entity id divided
 * by 16, i.e data of 16 consecutives ids will share the same block.
 *
 * Complexity is O(log(n/16)).
 */
class MapArray {
public:
    MapArray() {}; // default constructor
    virtual ~MapArray() {}; // default destructor

    // We reimplement some functions of the map interface

    /** \brief Same as std::map::at, except that no exception will be
     * thrown if the entity has not the composite.
     *
     * An exception will be thrown when trying to access a chunk that
     * was not initialized.
     *
     * It is the responsibility of the caller to ensure the validity of
      * the data by checking that the entity has the composite
     *
     * \param id the entity id
     * \return a reference on the data
     *
     */
    T& at(const id_t id) { return (DataChunk(id)).data[Index(id)]; };

    /** \brief Same as std::map::at const, except that no exception will
     * be thrown if the entity has not the composite.
     *
     * An exception will be thrown when trying to access a chunk that
     * was not initialized.
     *
     * It is the responsibility of the caller to ensure the validity of the
     * data by checking that the entity has the composite
     *
     * \param id the entity id
     * \return a const reference on the data
     *
     */
    const T& at(const id_t id) const { return at(id); };

    /** \brief Return a reference on the data for an entity
     *
     * The caller must check the validity of the data for this entity
     *
     * \param id const id_t the entity id
     * \return T& reference on the data
     *
     */
    T& operator[](const id_t id) {
        return (map_array[ChunkId(id)]).data[Index(id)];
    };

    /** \brief Erase the data for an entity
     *
     * Currently not implemented
     *
     * \param id const id_t the entity id
     *
     */
    void clear(const id_t id) {}; // TODO

    /** \brief Returns an iterator positioned on the first chunk
     *
     * \return MapArrayIterator<T> the iterator
     *
     */
    MapArrayIterator<T> begin() {
        return MapArrayIterator<T>(map_array.begin(), map_array.cend());
    };

    /** \brief Returns an iterator on the chunk past the end
     *
     * \return MapArrayIterator<T> the iterator
     *
     */
    MapArrayIterator<T> end() {
        return MapArrayIterator<T>(map_array.end());
    };

    /** \brief Return a reference on the chunk containing the data of
     * a specific entity
     *
     * \param id the id of the entity
     * \return Chunk<T>& the reference on the chunk
     *
     */
    Chunk<T>& DataChunk(const id_t id) {
        return map_array.at(ChunkId(id));
    };

    /** \brief Return a const reference on the chunk containing the data
     * of a specific entity
     *
     * \param id the id of the entity
     * \return const Chunk<T>& the const reference on the chunk
     *
     */
    const Chunk<T>& DataChunk(const id_t id) const {
        return DataChunk(id);
    };

    /** \brief Return the internal key that identifies the chunk in the map
     *
     * The key is the first bits of the id, without the last bits that
     * identifies the index in  the chunk
     *
     * the id can be computed using
     * id = (ChunkId(id) << CONTAINER_CHUNK_SHIFT) + Index(id)
     *
     * \param id const id_t the id of the entity
     * \return const chunk_id the internal key
     *
     */
    static const chunk_id ChunkId(const id_t id) {
        return id >> CONTAINER_CHUNK_SHIFT;
    };

    /** \brief Return the index of the data in the chunk for an entity
     *
     * This is the index in the internal array of the chunk.
     *
     * \param id const id_t the id of the entity
     * \return const size_t the index
     *
     */
    static const size_t Index(const id_t id) {
        return id & CONTAINER_CHUNK_MASK;
    };

private:
    // the map of arrays
    std::map<chunk_id, Chunk<T>> map_array;
};

template<class T>
/** \brief An iterator for MapArray
 */
class MapArrayIterator
        : public std::iterator<std::bidirectional_iterator_tag, T> {
public:
    /** \brief Default constructor
     *
     * \param std::map<id_t,Chunk<T>>::const_iterator& begin an iterator
     * that will be used as a starting point
     * \param std::map<id_t,Chunk<T>>::const_iterator& end an iterator on
     * the past-the-end element of the internal map
     *
     */
    MapArrayIterator(const typename std::map<id_t,
         Chunk<T>>::iterator& chunk_iterator,
         const typename std::map<id_t, Chunk<T>>::const_iterator& chunk_end)
                : chunk_iterator(chunk_iterator), chunk_end(chunk_end),
                tmp_pair(std::make_pair(0, std::ref(ref_T))) {
        if (chunk_iterator != chunk_end) {
            p = chunk_iterator->first << CONTAINER_CHUNK_SHIFT;
        }
    };

    /** \brief Return an iterator on the past-the-end element
     *
     * \param std::map<id_t,Chunk<T>>::const_iterator& end an iterator on
     * the past-the-end element of the internal map
     *
     */
    MapArrayIterator(const typename std::map<id_t, Chunk<T>>::iterator& chunk_end)
        : chunk_iterator(chunk_end), chunk_end(chunk_end),
        tmp_pair(std::make_pair(0, std::ref(ref_T))) {};

    /** \brief Default destructor
     *
     */
    virtual ~MapArrayIterator() {};

    /** \brief Copy constructor
     *
     */
    MapArrayIterator(const MapArrayIterator<T>& it)
        : chunk_iterator(it.chunk_iterator),
        chunk_end(it.chunk_end), tmp_pair(it.tmp_pair) {
        if (chunk_iterator != chunk_end) {
            p = it.p;
        }
    };

    // we override some operators
    MapArrayIterator& operator++() {
        if (chunk_iterator !=  chunk_end && 0 == MapArray<T>::Index(++p)) {
            ++chunk_iterator;
            p = chunk_iterator->first << CONTAINER_CHUNK_SHIFT;
        }
        return *this;
    };

    bool operator==(const MapArrayIterator<T>& mai) const {
        return (chunk_iterator == mai.chunk_iterator)
                && ((chunk_iterator == chunk_end) ||(p == mai.p));
    };

    bool operator!=(const MapArrayIterator<T>& mai) const {
        return ! (*this == mai);
    };

    std::pair<id_t,T>& operator*() {
        return std::make_pair(p, chunk_iterator->second.data[MapArray<T>::Index(p)]);
    };

    std::pair<id_t,std::reference_wrapper<T>>* operator->() {
        auto ref = &chunk_iterator->second.data[MapArray<T>::Index(p)];
        tmp_pair.first = p;
        tmp_pair.second = std::ref(*ref);
        return &tmp_pair;
    };

private:
    // post increment is private because I am lazy
    MapArrayIterator operator++(int) {};
    // cuurent position of the iterator
    id_t p;
    // iterators on the internal map
    typename std::map<id_t, Chunk<T>>::iterator chunk_iterator;
    typename std::map<id_t, Chunk<T>>::const_iterator chunk_end;
    T ref_T;
    // placeholder for returned value of dereference operator
    std::pair<id_t,std::reference_wrapper<T>> tmp_pair;
};
} // trillek

#endif // MAPARRAY_HPP_INCLUDED
