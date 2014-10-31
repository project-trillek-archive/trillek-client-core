#ifndef TRILLEK_H_INCLUDED
#define TRILLEK_H_INCLUDED

#include <stdint.h>
#include <list>
#include <memory>
#include "trillek-allocator.hpp"

// make_unique will be in C++14. Implemented here since we're using C++11.
// VS2013 already implements it, GCC 4.9 will implement it
// TODO: remove it when using GCC 4.9
#if defined(__GNUG__)
#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
using std::make_unique;
#endif


namespace trillek {

// type of an entity #id
typedef uint32_t id_t;

// type of a list
template<class U>
using trillek_list = std::list<U, TrillekAllocator<U>>;

/** \brief Type of component
 *
 * DYNAMIC: The component is passed to the system and then stored in the
 * ComponentFactory container
 * SYSTEM : The component is passed to the system that must store it.
 * SHARED : The component is passed to SharedComponent and is stored there
 *
 * Only SHARED components can be shared between systems in different threads.
 */
enum ComponentType { DYNAMIC, SYSTEM, SHARED };

namespace component {
enum class Component : uint32_t;
}

namespace reflection {

// Template methods that can be used for reflection.
// To use them just call GetTypeName<MyType>() to retrieve the name for the specified type.
template<class TYPE> const char* GetTypeName(void) { return "UNKNOWN"; }
template<class TYPE> unsigned int GetTypeID(void) { return ~0; }
// function to get the name of the handler of a packet type
template <uint32_t MAJOR,uint32_t MINOR> const char* GetNetworkHandler(void) {}

} // End of reflection
} // End fo trillek

#endif // TRILLEK_H_INCLUDED
