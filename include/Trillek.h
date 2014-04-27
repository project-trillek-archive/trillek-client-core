#ifndef TRILLEK_H_INCLUDED
#define TRILLEK_H_INCLUDED

#include <stdint.h>

// make_unique will be in C++14. Implemented here since we're using C++11.
// Commented because even VS2013 does not support var args
/*
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
*/
namespace trillek {

    // type of an entity #id
    typedef uint32_t id_t;

    namespace reflection {
        // Template methods that can be used for reflection.
        // To use them just call GetTypeName<MyType>() to retrieve the name for the specified type.
        template <typename TYPE> const char* GetTypeName(void) { }
        template <typename TYPE> const unsigned int GetTypeID(void) { }
    }
}
#endif // TRILLEK_H_INCLUDED
