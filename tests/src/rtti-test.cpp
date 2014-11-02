#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include "property.hpp"
#include "type-id.hpp"
#include "graphics/graphics-container.hpp"

namespace trillek {
namespace reflection {
//template<> const char* GetTypeName<int>() { return "int"; }
//template<> inline const unsigned int GetTypeID<int32_t>() { return 4; }
}
}

#include <vector>

namespace trillek {

#undef TRILLEK_MAKE_IDTYPE
#undef TRILLEK_MAKE_IDTYPE_NS
#define TRILLEK_MAKE_IDTYPE(t,n) EXPECT_EQ(n, reflection::GetTypeID<t>()); EXPECT_EQ(std::string(#t), std::string(reflection::GetTypeName<t>()));
#define TRILLEK_MAKE_IDTYPEN(t,n) EXPECT_EQ(n, reflection::GetTypeID<t>());
#define TRILLEK_MAKE_IDTYPE_NS(ns,t,n) EXPECT_EQ(n, reflection::GetTypeID<ns::t>()); EXPECT_EQ(std::string(#t), std::string(reflection::GetTypeName<ns::t>()));

struct test_datatype {
    unsigned a, b, c;
};

TEST(RTTITests, TypeIDMatch) {
    EXPECT_EQ(std::string("UNKNOWN"), std::string(reflection::GetTypeName<test_datatype>()));
    TRILLEK_MAKE_IDTYPE(void, 0)
    TRILLEK_MAKE_IDTYPE(void*, 1)
    TRILLEK_MAKE_IDTYPE(char, 2)
    TRILLEK_MAKE_IDTYPEN(short, 3)
    TRILLEK_MAKE_IDTYPEN(int, 4)
    TRILLEK_MAKE_IDTYPE(unsigned char, 7)
    TRILLEK_MAKE_IDTYPEN(unsigned short, 8)
    TRILLEK_MAKE_IDTYPEN(unsigned int, 9)
    TRILLEK_MAKE_IDTYPE(bool, 12)

    TRILLEK_MAKE_IDTYPE(int8_t, 2)
    TRILLEK_MAKE_IDTYPE(int16_t, 3)
    TRILLEK_MAKE_IDTYPE(int32_t, 4)
    TRILLEK_MAKE_IDTYPE(int64_t, 5)
    TRILLEK_MAKE_IDTYPEN(uint8_t, 7)
    TRILLEK_MAKE_IDTYPE(uint16_t, 8)
    TRILLEK_MAKE_IDTYPE(uint32_t, 9)
    TRILLEK_MAKE_IDTYPE(uint64_t, 10)

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
}

TEST(RTTITests, BasicTypes) {
    using graphics::Container;

    const std::string name = "Test";
    std::vector<Container> test_vector;
    int i = 0;

    test_vector.push_back(Container((int)10));
    EXPECT_EQ(reflection::GetTypeID<int>(), test_vector[i].GetType());
    EXPECT_NE(reflection::GetTypeID<test_datatype>(), test_vector[i].GetType());
    EXPECT_EQ(sizeof(int), test_vector[i].GetSize());
    EXPECT_TRUE(test_vector[i].Is<int>());
    EXPECT_FALSE(test_vector[i].Is<int64_t>());
    EXPECT_FALSE(test_vector[i].Is<unsigned int>());
    EXPECT_FALSE(test_vector[i].Is<test_datatype>());

    test_vector.push_back(Container((int64_t)10)); i++;
    EXPECT_EQ(reflection::GetTypeID<int64_t>(), test_vector[i].GetType());
    EXPECT_NE(reflection::GetTypeID<test_datatype>(), test_vector[i].GetType());
    EXPECT_EQ(sizeof(int64_t), test_vector[i].GetSize());
    EXPECT_TRUE(test_vector[i].Is<int64_t>());
    EXPECT_FALSE(test_vector[i].Is<int>());
    EXPECT_FALSE(test_vector[i].Is<unsigned long>());
    EXPECT_FALSE(test_vector[i].Is<test_datatype>());

    test_vector.push_back(Container(std::string("test"))); i++;
    EXPECT_EQ(reflection::GetTypeID<std::string>(), test_vector[i].GetType());
    EXPECT_NE(reflection::GetTypeID<test_datatype>(), test_vector[i].GetType());
    EXPECT_TRUE(test_vector[i].Is<std::string>());
    EXPECT_FALSE(test_vector[i].Is<long>());
    EXPECT_FALSE(test_vector[i].Is<double>());
    EXPECT_FALSE(test_vector[i].Is<test_datatype>());
}

}
