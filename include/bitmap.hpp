#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

#include <vector>
#include <stdexcept>
#include <memory>
#include "util/utiltype.hpp"
#include "logging.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#define ROUND_DOWN(x, s) ((x) & ~((s)-1)) // rounds down x to a multiple of s (i.e. ROUND_DOWN(5, 4) becomes 4)

namespace trillek {
/** \brief A reference to simulate an lvalue
 */
template<class T>
class reference {
public:
    reference(T& c, const size_t offset) : c(c), offset(offset) {};

    reference(reference& r) : c(r.c) {
        c = r.c;
        offset = offset;
    }

    reference(reference&& r) : c(r.c) {
        offset = std::move(offset);
    }

    virtual ~reference() {};

    reference& operator=(bool b) {
        T m(T(1) << offset);
        c = (c & ~m) | (-b & m);
        if (b)
        return *this;
    };

    reference& operator=(reference& r) {
        c = r.c;
        offset = offset;
        return *this;
    }

    reference& operator=(reference&& r) {
        c = std::move(r.c);
        offset = std::move(offset);
        return *this;
    }

    reference& operator|=(bool b) {
        if (b) {
            c |= T(1) << offset;
        }
        return *this;
    };

    reference& operator&=(bool b) {
        if (! b) {
            c &= ~ (T(1) << offset);
        }
        return *this;
    };

    operator bool() const {
        return (c & (T(1) << offset)) != 0;
    };

private:
    T& c;
    size_t offset;
};

template<class T>
class BitMapEnumerator;

/** \brief A bitset like boost::dynamic_bitset
 */
template<class T>
class BitMap {
public:
    /** \brief Constructor of a BitSet
     *
     */
    // Default constructor
    BitMap() : BitMap(0, false) {};
    // Constructor with default value
    BitMap(const bool b) : BitMap(0,b) {};
    // Constructor with initial size
    BitMap(const size_t s) : BitMap(s, false) {};
    // Constructor with initial size and default value
    BitMap(const size_t s, const bool b) :
            bsize(s), def_value(b ? -1 : 0),
                                     last_block(0), first_block(0) {};

    // Default destructor
    virtual ~BitMap() {};

    // Copy constructor
    BitMap(const BitMap& ba) {
        bitarray = ba.bitarray;
        def_value = ba.def_value;
        first_block = ba.first_block;
        last_block = ba.last_block;
        bsize = ba.bsize;
    }
    // Move Constructor
    BitMap(BitMap&& ba) {
        bitarray = std::move(ba.bitarray);
        def_value = std::move(ba.def_value);
        first_block = std::move(ba.first_block);
        last_block = std::move(ba.last_block);
        bsize = std::move(ba.bsize);
    }
    // Copy assignment
    BitMap& operator=(const BitMap& ba) {
        bitarray = ba.bitarray;
        def_value = ba.def_value;
        first_block = ba.first_block;
        last_block = ba.last_block;
        bsize = ba.bsize;
        return *this;
    }
    // Move assignment
    BitMap& operator=(BitMap&& ba) {
        bitarray = std::move(ba.bitarray);
        def_value = std::move(ba.def_value);
        first_block = std::move(ba.first_block);
        last_block = std::move(ba.last_block);
        bsize = std::move(ba.bsize);
        return *this;
    }

    // Compound assignment operators
    // OR combination between 2 BitSets
    BitMap& operator|=(const BitMap& ba) {
        MixArray(*this, ba, lambda_OR);
        return *this;
    }

    // AND combination between 2 BitSets
    BitMap& operator&=(const BitMap& ba) {
        MixArray(*this, ba, lambda_AND);
        return *this;
    }

    // XOR combination between 2 BitSets
    BitMap& operator^=(const BitMap& ba) {
        MixArray(*this, ba, lambda_XOR);
        return *this;
    }

    // NOT operation
    BitMap<T> operator~() const {
        BitMap<T> ret;
        ret.first_block = this->first_block;
        ret.last_block = this->last_block;
        ret.bsize = this->bsize;
        ret.bitarray.reserve(this->bitarray.size());
        for (auto& c : this->bitarray) {
           ret.bitarray.push_back(~c);
        }
        ret.def_value = ~this->def_value;
        return ret;
    }

    // Access to an element of the BitSet
    bool at(size_t idx) const {
        auto offset = idx / BlockSize();
        if (offset >= size()) {
            throw new std::out_of_range("Index does not exist");
        }
        if (offset >= last_block || offset < first_block) {
            return def_value;
        }
        auto bit_id = idx % BlockSize();
        return ((bitarray[offset - first_block] & (T(1) << bit_id)) != 0);
    }

    // left-side reference
    reference<T> operator[](size_t idx) {
        auto offset = idx / BlockSize();
        if (offset >= last_block) {
            if (! last_block) {
                first_block = offset;
            }
            bitarray.resize(offset - first_block + 1, def_value);
            last_block = offset + 1;
        }
        if (idx >= bsize) {
            bsize = idx + 1;
        }
        else if (offset < first_block) {
            auto bitarray2 = std::vector<T>(last_block - offset, def_value);
            auto temp = first_block - offset;
            for (auto i = 0; i < bitarray.size(); ++i) {
                bitarray2[temp + i] = std::move(bitarray[i]);
            }
            bitarray = std::move(bitarray2);
            first_block = offset;
        }
        auto bit_id = idx % BlockSize();
        return reference<T>(bitarray[offset - first_block], bit_id);
    }

    void erase(size_t idx) {
        auto offset = idx / BlockSize();
        if (offset < last_block && offset >= first_block) {
            (*this)[idx] = def_value;
        }
    }

    const size_t size() const {
        return bsize;
    }

    const T* data() const {
        return bitarray.data();
    }

    BitMapEnumerator<T> enumerator(size_t max_iterations) const {
        return BitMapEnumerator<T>(*this, max_iterations);
    };

    const size_t countTrue() const {
        if (first_block == last_block) {
            return def_value ? size() : 0;
        }
        size_t sum = def_value ? first_block * BlockSize() : 0;
        const auto length = size();
        const auto last_index = (std::min)(length, last_block * BlockSize());
        #if defined(__GNUG__) || defined(_MSC_VER)
            const auto start = bitarray.data();
            const auto end = start + ((last_index- first_block * BlockSize()) >> util::Log2Bin<T>());
            auto i = start;
            for (; i < end; ++i) {
                sum += util::PopCount<T>(*i);
            }
            sum += util::PopCount<T>(*i & ( (T(1) << (last_index & (BlockSize() - 1))) -1));
        #else
            size_t j = first_block << BlockSize();
            for (; j < last_index; ++j) {
                if ((*this)[j]) {
                    ++sum;
                }
            }
        #endif
        if (length >= last_block * BlockSize()) {
            sum += def_value ? (length  - last_block * BlockSize()) : 0;
        }
        return sum;
    }

    size_t LastBlock() const {
        return last_block;
    }

    size_t FirstBlock() const {
        return first_block;
    }

    size_t BlockSize() const {
        return (sizeof(T) << 3);
    }

    bool DefaultValue() const {
        return def_value;
    }

private:
    void MixArray(BitMap<T>& a, const BitMap<T>& b, const std::function<T(const T&,const T&)>& operation) {
        auto left = std::min(a.first_block, b.first_block);
        auto right = std::max(a.last_block, b.last_block);
        auto& first_left = (left == a.first_block ? a : b);
        auto& last_left = (&first_left == &b ? a : b);
        auto& last_right = (right == a.last_block ? a : b);
        auto& first_right = (&last_right == &b ? a : b);
        auto middle_left = std::min(last_left.first_block, first_right.last_block);
        auto middle_right = std::max(last_left.first_block, first_right.last_block);
        std::vector<T> result;
        result.reserve(right - left);
        auto first_size = middle_left - left;
        auto last_size = right - middle_right;
        for (size_t j = 0; j < first_size; ++j) {
            result.push_back(operation(first_left.bitarray.at(j), last_left.def_value));
        }
        if (first_size || last_size) {
            if (middle_left == last_left.first_block) {
                for(size_t j = first_size, k = 0 ; k < middle_right - middle_left; ++k, ++j) {
                    result.push_back(operation(first_left.bitarray.at(j), last_left.bitarray.at(k)));
                }
            }
            else if (first_size && last_size){
                for(size_t k = 0 ; k < middle_right - middle_left; ++k) {
                    result.push_back(operation(first_left.def_value, last_left.def_value));
                }
            }
        }
        for (size_t j = middle_right - left, k = last_right.bitarray.size() - last_size; k < last_right.bitarray.size(); ++j, ++k) {
            result.push_back(operation(last_right.bitarray.at(k), first_right.def_value));
        }
        a.def_value = operation(a.def_value, b.def_value);
        if(result.size()) {
            a.first_block = first_size ? left : last_left.first_block;
            a.last_block = last_size ? right : first_right.last_block;
        }
        else {
            a.first_block = 0;
            a.last_block = 0;
        }
        a.bsize = std::max(a.bsize, b.bsize);
        a.bitarray = std::move(result);
    }

    const std::function<T(const T&,const T&)> lambda_OR = [](const T& a, const T& b) { return a | b;};
    const std::function<T(const T&,const T&)> lambda_AND = [](const T& a, const T& b) { return a & b;};
    const std::function<T(const T&,const T&)> lambda_XOR = [](const T& a, const T& b) { return a ^ b;};


    std::vector<T> bitarray;
    // number of elements
    size_t bsize;
    // index of first block
    size_t first_block;
    // index of "after" last block
    size_t last_block;
    T def_value;
};

// Bitwise logical operators
template<class T>
BitMap<T> operator&(const BitMap<T>& lhs, const BitMap<T>& rhs) {
    auto ret = lhs;
    ret &= rhs;
    return ret;
}

template<class T>
BitMap<T> operator|(const BitMap<T>& lhs, const BitMap<T>& rhs) {
    auto ret = lhs;
    ret |= rhs;
    return ret;
}

template<class T>
BitMap<T> operator^(const BitMap<T>& lhs, const BitMap<T>& rhs) {
    auto ret = lhs;
    ret ^= rhs;
    return ret;
}

#if defined(__GNUG__) || defined(_MSC_VER) // define BitMapEnumerator per compiler
template<class T>
class BitMapEnumerator {
public:
    BitMapEnumerator(const BitMap<T>& bs, size_t max_iterations) : bitarray(bs),
        current_long((T*) bs.data()), max_iterations(max_iterations),
        last_bit(0), current_value(-1) { ++(*this); };
    virtual ~BitMapEnumerator() {};

    size_t operator++() {
        auto length = std::max(bitarray.size() + BlockSize(),max_iterations);
        const auto last_index = std::min(length, bitarray.LastBlock() * BlockSize());
        const auto end = start + ((last_index) >> util::Log2Bin<T>());
        const auto end_1void = bitarray.FirstBlock() << util::Log2Bin<T>();
        if(++current_value < end_1void) {
            if (bitarray.DefaultValue()) {
                return current_value;
            }
            current_value = end_1void;
        }
        for (; current_long < end ; current_long++) {
            if (*current_long && last_bit < BlockSize()) {
                T tmp(*current_long >> last_bit);
                if (util::PopCount<T>(tmp)) {
                    last_bit += util::Ctz<T>(tmp);
                    current_value = last_bit++ + ((current_long - start + bitarray.FirstBlock()) << util::Log2Bin<T>());
                    return current_value;
                }
            }
            else {
                last_bit = 0;
            }
        }
        if(current_value < length) {
            if (bitarray.DefaultValue()) {
                return current_value;
            }
            current_value = length;
        }
        return current_value;
    };

    const size_t operator*() const {
        return current_value;
    }

    size_t BlockSize() {
        return (sizeof(T) << 3);
    }

private:
    int last_bit;
    T* current_long;
    const T* const start = current_long;
    const BitMap<T>& bitarray;
    size_t current_value;
    size_t max_iterations;
};

#else // other compilers : not optimized at all

template<class T>
class BitMapEnumerator {
public:
    BitMapEnumerator(BitMap<T>& bs, size_t max_iterations)
     : bitarray(bs), current_value(-1), max_iterations(max_iterations) { ++(*this); };
    virtual ~BitMapEnumerator() {};

    size_t operator++() {
        auto length = bitarray.size();
        auto length2 = std::max(bitarray.size(),max_iterations);
        for (++current_value; current_value < length; ++current_value) {
            if (bitarray.at(current_value)) {
                return current_value;
            }
        }
        if(current_value < length2) {
            if (bitarray.DefaultValue()) {
                return current_value;
            }
            current_value = length2;
        }
        return current_value;
    };

    const size_t operator*() const {
        return current_value;
    }

private:
    BitMap<T>& bitarray;
    size_t current_value;
};
#endif
}

#endif // BITMAP_H_INCLUDED
