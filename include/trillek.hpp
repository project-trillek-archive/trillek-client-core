#ifndef TRILLEK_H_INCLUDED
#define TRILLEK_H_INCLUDED

#include <stdint.h>

namespace trillek {

// type of an entity #id
typedef uint32_t id_t;

namespace reflection {

// Template methods that can be used for reflection.
// To use them just call GetTypeName<MyType>() to retrieve the name for the specified type.
template<class TYPE> const char* GetTypeName(void) { return "UNKNOWN"; }
template<class TYPE> const unsigned int GetTypeID(void) { return ~0; }

} // End of reflection
} // End fo trillek

#endif // TRILLEK_H_INCLUDED
