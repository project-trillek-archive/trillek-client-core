#ifndef UTILITY_TEST_H_INCLUDED
#define UTILITY_TEST_H_INCLUDED

#include <gtest/gtest.h>

#include "util/utiltype.hpp"
#include "util/checksum.hpp"

namespace trillek {
namespace util {

TEST(UtilTest, BitFlip) {
    uint32_t nb32 = BitReverse32(0x12345678);
    EXPECT_EQ(0x1E6A2C48, nb32);
    nb32 = BitReverse(0xA4C, 10);
    EXPECT_EQ(0xC9, nb32);
    uint16_t nb16 = BitReverse16(0x1234);
    EXPECT_EQ(0x2C48, nb16);
}

TEST(UtilTest, FourCC) {
    EXPECT_EQ(FourCC('T','e','S','t'), FourCC("TeSt"));
}

TEST(UtilTest, Checksums) {
    std::string tqbf = std::string(
        "The quick brown fox jumps over the lazy dog"
        );
    // test string is the SHA-512 of The quick brown fox... text
    std::string tqbf512 = std::string(
        "07e547d9586f6a73f73fbac0435ed769"
        "51218fb7d0c8d788a309d785436bbb64"
        "2e93a252a954f23912547d1e8a3b5ed6"
        "e1bfd7097821233fa0538f3db854fee6"
        );
    algorithm::Crc32 crc;
    crc.Full(tqbf);
    EXPECT_EQ(0x414fa339, crc.ldata);
    crc.Full(tqbf512);
    EXPECT_EQ(0xfb213500, crc.ldata);

    algorithm::Adler32 adler;
    adler.Full(tqbf);
    EXPECT_EQ(0x5bdc0fda, adler.ldata);
    adler.Full(tqbf512);
    EXPECT_EQ(0x928b2243, adler.ldata);
}

static ErrorReturn<void> T_NoError() {
    return ErrorReturn<void>(0, "NO ERROR");
}
static ErrorReturn<void> T_NoErrorVDef() {
    return ErrorReturn<void>();
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
static ErrorReturn<std::string> T_NoErrorRetDef() {
    return ErrorReturn<std::string>("test");
}
static ErrorReturn<std::string> T_NoErrorRetNoMsg() {
    return ErrorReturn<std::string>("test", 0);
}
static ErrorReturn<std::string> T_ErrorRetNoMsg() {
    return ErrorReturn<std::string>("test", 5);
}
TEST(UtilTest, ErrorReturnClass) {
    EXPECT_EQ(false, ((bool)T_NoErrorRet()));
    EXPECT_EQ(true, ((bool)T_ErrorRet()));
    EXPECT_EQ(false, ((bool)T_NoErrorRetDef()));

    ErrorReturn<std::string> sts = ErrorReturn<std::string>("");

    EXPECT_EQ(std::string("test"), T_NoErrorRetDef().value);
    EXPECT_EQ(std::string(""), T_NoErrorRetDef().error_text);
    EXPECT_EQ(0, T_NoErrorRetDef().error_code);

    EXPECT_EQ(std::string("test"), *T_NoErrorRet());

    EXPECT_EQ(std::string("test"), (sts = T_NoErrorRet()).value);
    EXPECT_EQ(std::string("NO ERROR"), sts.error_text);
    EXPECT_EQ(0, sts.error_code);
    EXPECT_EQ(false, ((bool)sts));

    EXPECT_EQ(std::string("test"), (sts = T_ErrorRet()).value);
    EXPECT_EQ(std::string("ERROR 5"), sts.error_text);
    EXPECT_EQ(5, sts.error_code);
    EXPECT_EQ(true, ((bool)sts));

    EXPECT_EQ(std::string("test"), (sts = T_NoErrorRetNoMsg()).value);
    EXPECT_EQ(std::string(""), sts.error_text);
    EXPECT_EQ(0, sts.error_code);
    EXPECT_EQ(false, ((bool)sts));

    EXPECT_EQ(std::string("test"), (sts = T_ErrorRetNoMsg()).value);
    EXPECT_EQ(std::string(""), sts.error_text);
    EXPECT_EQ(5, sts.error_code);
    EXPECT_EQ(true, ((bool)sts));
}
TEST(UtilTest, ErrorReturnVoidClass) {
    EXPECT_EQ(false, ((bool)T_NoErrorVDef()));
    ErrorReturn<void> st;

    EXPECT_EQ(std::string("NO ERROR"), (st = T_NoError()).error_text);
    EXPECT_EQ(std::string("NO ERROR"), st.error_text);
    EXPECT_EQ(0, st.error_code);
    EXPECT_EQ(false, ((bool)st));

    EXPECT_EQ(std::string("ERROR 5"), (st = T_Error()).error_text);
    EXPECT_EQ(std::string("ERROR 5"), st.error_text);
    EXPECT_EQ(5, st.error_code);
    EXPECT_EQ(true, ((bool)st));

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
