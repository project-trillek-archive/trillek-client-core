#ifndef TYPE_ID_HPP_INCLUDED
#define TYPE_ID_HPP_INCLUDED

#include "trillek.hpp"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <string>

namespace trillek {
class Property;

namespace graphics {
class Container;
}

namespace reflection {

// some macros to make definitions a bit easier to read
#define TRILLEK_MAKE_IDTYPE(a,b) \
    template<> inline const char* GetTypeName<a>() { return #a; } \
    template<> inline unsigned int GetTypeID<a>() { return b; }

#define TRILLEK_MAKE_IDTYPE_NAME(a,n,b) \
    template<> inline const char* GetTypeName<a>() { return n; } \
    template<> inline unsigned int GetTypeID<a>() { return b; }

// define something in a namespace, without the namespace in the type name
#define TRILLEK_MAKE_IDTYPE_NS(ns,a,b) \
    template<> inline const char* GetTypeName<ns::a>() { return #a; } \
    template<> inline unsigned int GetTypeID<ns::a>() { return b; }

// various generic types
TRILLEK_MAKE_IDTYPE(void, 0)
TRILLEK_MAKE_IDTYPE(void*, 1)
TRILLEK_MAKE_IDTYPE(char, 2)
TRILLEK_MAKE_IDTYPE(int16_t, 3)
TRILLEK_MAKE_IDTYPE(int32_t, 4)
TRILLEK_MAKE_IDTYPE(int64_t, 5)
//TRILLEK_MAKE_IDTYPE(__int128, 6)
TRILLEK_MAKE_IDTYPE(unsigned char, 7)
TRILLEK_MAKE_IDTYPE(uint16_t, 8)
TRILLEK_MAKE_IDTYPE(uint32_t, 9)
TRILLEK_MAKE_IDTYPE(uint64_t, 10)
//TRILLEK_MAKE_IDTYPE(unsigned __int128, 11)
TRILLEK_MAKE_IDTYPE(bool, 12)

TRILLEK_MAKE_IDTYPE(int8_t, 2)
//TRILLEK_MAKE_IDTYPE(int16_t, 3)
//TRILLEK_MAKE_IDTYPE(int32_t, 4)
//TRILLEK_MAKE_IDTYPE(int64_t, 6)
//TRILLEK_MAKE_IDTYPE(uint8_t, 7)
//TRILLEK_MAKE_IDTYPE(uint16_t, 8)
//TRILLEK_MAKE_IDTYPE(uint32_t, 9)
//TRILLEK_MAKE_IDTYPE(uint64_t, 11)

TRILLEK_MAKE_IDTYPE(float, 20)
TRILLEK_MAKE_IDTYPE(double, 21)
TRILLEK_MAKE_IDTYPE(long double, 22)
TRILLEK_MAKE_IDTYPE_NS(glm, vec2, 23)
TRILLEK_MAKE_IDTYPE_NS(glm, vec3, 24)
TRILLEK_MAKE_IDTYPE_NS(glm, vec4, 25)
TRILLEK_MAKE_IDTYPE_NS(glm, mat2x2, 26)
TRILLEK_MAKE_IDTYPE_NS(glm, mat3x3, 27)
TRILLEK_MAKE_IDTYPE_NS(glm, mat4x4, 28)
TRILLEK_MAKE_IDTYPE_NS(glm, quat, 29)
TRILLEK_MAKE_IDTYPE(graphics::Container, 30)
TRILLEK_MAKE_IDTYPE(Property, 31)
TRILLEK_MAKE_IDTYPE_NS(std, string, 32)
TRILLEK_MAKE_IDTYPE_NS(glm, dvec2, 33)
TRILLEK_MAKE_IDTYPE_NS(glm, dvec3, 34)
TRILLEK_MAKE_IDTYPE_NS(glm, dvec4, 35)
TRILLEK_MAKE_IDTYPE_NS(glm, dmat2x2, 36)
TRILLEK_MAKE_IDTYPE_NS(glm, dmat3x3, 37)
TRILLEK_MAKE_IDTYPE_NS(glm, dmat4x4, 38)
TRILLEK_MAKE_IDTYPE_NS(glm, dquat, 39)

}// namespace reflection
} // namespace trillek

#endif
