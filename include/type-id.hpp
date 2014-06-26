#ifndef TYPE_ID_HPP_INCLUDED
#define TYPE_ID_HPP_INCLUDED

#include "trillek.hpp"
#include <string>

class Property;

namespace trillek {
class Container;

namespace reflection {

// make definitions a bit easier to read
#define TRILLEK_MAKE_IDTYPE(a,b) \
    template<> const char* GetTypeName<a>() { return #a; } \
    template<> const unsigned int GetTypeID<a>() { return b; }

// make definitions a bit easier to read
#define TRILLEK_MAKE_IDTYPE_NAME(a,n,b) \
    template<> const char* GetTypeName<a>() { return n; } \
    template<> const unsigned int GetTypeID<a>() { return b; }

// various generic types
TRILLEK_MAKE_IDTYPE(void, 0);
TRILLEK_MAKE_IDTYPE(void*, 1);
TRILLEK_MAKE_IDTYPE(int8_t, 2);
TRILLEK_MAKE_IDTYPE(int16_t, 3);
TRILLEK_MAKE_IDTYPE(int32_t, 4);
TRILLEK_MAKE_IDTYPE(long, 5);
TRILLEK_MAKE_IDTYPE(int64_t, 6);
TRILLEK_MAKE_IDTYPE(bool, 7);

TRILLEK_MAKE_IDTYPE(uint8_t, 10);
TRILLEK_MAKE_IDTYPE(uint16_t, 11);
TRILLEK_MAKE_IDTYPE(uint32_t, 12);
TRILLEK_MAKE_IDTYPE(unsigned long, 13);
TRILLEK_MAKE_IDTYPE(uint64_t, 14);

TRILLEK_MAKE_IDTYPE(float, 20);
TRILLEK_MAKE_IDTYPE(double, 21);
TRILLEK_MAKE_IDTYPE(long double, 22);

TRILLEK_MAKE_IDTYPE(Container, 30);
TRILLEK_MAKE_IDTYPE(Property, 31);
TRILLEK_MAKE_IDTYPE_NAME(std::string, "string", 32);

}// namespace reflection
} // namespace trillek

#endif
