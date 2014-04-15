#ifndef TRILLEK_H_INCLUDED
#define TRILLEK_H_INCLUDED

#include <stdint.h>

// make_unique will be in C++14. Implemented here since we're using C++11.
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

namespace trillek {

    // type of an entity #id
    typedef uint32_t id_t;

}
#endif // TRILLEK_H_INCLUDED
