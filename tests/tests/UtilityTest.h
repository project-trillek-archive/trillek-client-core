#ifndef UTILITY_TEST_H_INCLUDED
#define UTILITY_TEST_H_INCLUDED

#include <gtest/gtest.h>

#include "util/UtilType.hpp"
#include "util/Checksum.hpp"

namespace trillek {
namespace util {

    TEST(UtilTest, BitFlip32) {
        uint32_t nb = BitReverse32(0x12345678);
        EXPECT_EQ(0x1E6A2C48, nb);
    }
    TEST(UtilTest, BitFlip16) {
        uint16_t nb = BitReverse16(0x1234);
        EXPECT_EQ(0x2C48, nb);
    }

    static ErrorReturn<void> T_NoError() {
        return ErrorReturn<void>(0, "NO ERROR");
    }
    static ErrorReturn<void> T_Error() {
        return ErrorReturn<void>(5, "ERROR 5");
    }
    static ErrorReturn<std::string> T_NoErrorRet() {
        return ErrorReturn<std::string>("test", 0, "NO ERROR");
    }
    static ErrorReturn<std::string> T_ErrorRet() {
        return ErrorReturn<std::string>("test", 5, "ERROR 5");
    }
    TEST(UtilTest, ErrorReturnType) {
        EXPECT_EQ(false, ((bool)T_NoError()) );
        EXPECT_EQ(true, ((bool)T_Error()) );

        ErrorReturn<void> st;

        EXPECT_EQ(std::string("test"), (st = T_NoErrorRet()));
        EXPECT_EQ(std::string("NO ERROR"), st.error_text);
        EXPECT_EQ(0, st.error_code);
        EXPECT_EQ(false, ((bool)st));

        EXPECT_EQ(std::string("test"), (st = T_ErrorRet()));
        EXPECT_EQ(std::string("ERROR 5"), st.error_text);
        EXPECT_EQ(5, st.error_code);
        EXPECT_EQ(true, ((bool)st));
    }
}
}

#endif
