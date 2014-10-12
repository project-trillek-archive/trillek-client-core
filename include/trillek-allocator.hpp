#ifndef TRILLEKALLOCATOR_HPP_INCLUDED
#define TRILLEKALLOCATOR_HPP_INCLUDED

#include <cstddef>

extern size_t gAllocatedSize;

namespace trillek {

template<typename T>
class TrillekAllocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    /// Define a global that holds the allocated size

    /// Default constructor
    TrillekAllocator() throw() { }
    /// Copy constructor
    TrillekAllocator(const TrillekAllocator&) throw() { }
    /// Copy constructor with another type
    template<typename U>
    TrillekAllocator(const TrillekAllocator<U>&) throw() { }

    /// Destructor
    ~TrillekAllocator() { }

    /// Copy
    TrillekAllocator<T>& operator=(const TrillekAllocator&) {
        return *this;
    }
    /// Copy with another type
    template<typename U>
    TrillekAllocator& operator=(const TrillekAllocator<U>&) {
        return *this;
    }

    bool operator==(const TrillekAllocator& lhs) {
        return true;
    }

    /// Get address of reference
    pointer address(reference x) const {
        return &x;
    }
    /// Get const address of const reference
    const_pointer address(const_reference x) const {
        return &x;
    }

    /// Allocate memory
    pointer allocate(size_type n, const void* = 0) {
        size_type size = n * sizeof(value_type);
        gAllocatedSize += size;
        return (pointer)::operator new(size);
    }

    /// Deallocate memory
    void deallocate(void* p, size_type n) {
        size_type size = n * sizeof(T);
        gAllocatedSize -= size;
        ::operator delete(p);
    }

    /// Call constructor
    void construct(pointer p, const T& val) {
        // Placement new
        new ((T*)p) T(val);
    }
    /// Call constructor with more arguments
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        // Placement new
        ::new((void*)p) U(std::forward<Args>(args)...);
    }

    /// Call the destructor of p
    void destroy(pointer p) {
        p->~T();
    }
    /// Call the destructor of p of type U
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    /// Get the max allocation size
    size_type max_size() const {
        return size_type(-1);
    }

    /// A struct to rebind the allocator to another allocator of type U
    template<typename U>
    struct rebind {
        typedef TrillekAllocator<U> other;
    };
};

}

#endif // TRILLEKALLOCATOR_HPP_INCLUDED
