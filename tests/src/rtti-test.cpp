#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include "property.hpp"
#include "type-id.hpp"

namespace trillek {
namespace reflection {
//template<> const char* GetTypeName<int>() { return "int"; }
//template<> inline const unsigned int GetTypeID<int32_t>() { return 4; }
}
}

#include <vector>

namespace trillek {

#undef TRILLEK_MAKE_IDTYPE
#define TRILLEK_MAKE_IDTYPE(t,n) EXPECT_EQ(n, reflection::GetTypeID<t>()); EXPECT_EQ(std::string(#t), std::string(reflection::GetTypeName<t>()));
#define TRILLEK_MAKE_IDTYPEN(t,n) EXPECT_EQ(n, reflection::GetTypeID<t>());

struct test_datatype {
    unsigned a, b, c;
};

TEST(RTTITests, TypeIDMatch) {
    EXPECT_EQ(std::string("UNKNOWN"), std::string(reflection::GetTypeName<int>()));
    EXPECT_EQ(std::string("UNKNOWN"), std::string(reflection::GetTypeName<test_datatype>()));
    TRILLEK_MAKE_IDTYPE(void, 0)
    TRILLEK_MAKE_IDTYPE(void*, 1)
    TRILLEK_MAKE_IDTYPE(char, 2)
    TRILLEK_MAKE_IDTYPE(short, 3)
    TRILLEK_MAKE_IDTYPEN(int, ~0)
    TRILLEK_MAKE_IDTYPE(long, 5)
    TRILLEK_MAKE_IDTYPE(long long, 6)
    TRILLEK_MAKE_IDTYPE(unsigned char, 7)
    TRILLEK_MAKE_IDTYPE(unsigned short, 8)
    TRILLEK_MAKE_IDTYPE(unsigned int, 9)
    TRILLEK_MAKE_IDTYPE(unsigned long, 10)
    TRILLEK_MAKE_IDTYPE(unsigned long long, 11)
    TRILLEK_MAKE_IDTYPE(bool, 12)
    TRILLEK_MAKE_IDTYPE(int8_t, 2)
    TRILLEK_MAKE_IDTYPEN(int16_t, 3)
    TRILLEK_MAKE_IDTYPEN(int32_t, ~0)
    TRILLEK_MAKE_IDTYPEN(int64_t, 6)
    TRILLEK_MAKE_IDTYPEN(uint8_t, 7)
    TRILLEK_MAKE_IDTYPEN(uint16_t, 8)
    TRILLEK_MAKE_IDTYPEN(uint32_t, 9)
    TRILLEK_MAKE_IDTYPEN(uint64_t, 11)
}

TEST(RTTITests, BasicTypes) {
    const std::string name = "Test";
    long testInt = 10;
    std::vector<Property> test_vector;
    Property p_int(name, testInt);
    test_vector.push_back(p_int);
    Property p_str(name, std::string("test"));
    test_vector.push_back(p_str);
    EXPECT_EQ(reflection::GetTypeID<long>(), test_vector[0].GetType());
    EXPECT_EQ(sizeof(long), test_vector[0].GetSize());
    EXPECT_TRUE(test_vector[0].Is<long>());
    EXPECT_FALSE(test_vector[0].Is<unsigned long>());
    EXPECT_TRUE(test_vector[1].Is<std::string>());
    EXPECT_FALSE(test_vector[1].Is<long>());
    EXPECT_FALSE(test_vector[1].Is<double>());
}

}
