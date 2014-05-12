#ifndef DECOMPRESSOR_TEST_H_INCLUDED
#define DECOMPRESSOR_TEST_H_INCLUDED

#include <gtest/gtest.h>

#include "util/Compression.hpp"

namespace trillek {
namespace util {
namespace algorithm {

    TEST(DecompressorTest, BitBuffering) {
        const unsigned char testdata[] = { 0xA5u, 0x4Eu };
        BitStreamDecoder inf;
        void_er st;
        EXPECT_EQ(2, sizeof(testdata));
        inf.indata.append(testdata, sizeof(testdata));
        EXPECT_EQ(0x5, st = inf.GetBits(4));
        EXPECT_EQ(std::string(), st.error_text);
        EXPECT_EQ(0xA, st = inf.GetBits(4));
        EXPECT_EQ(std::string(), st.error_text);
        EXPECT_EQ(0x2, st = inf.GetBits(2));
        EXPECT_EQ(std::string(), st.error_text);
        EXPECT_EQ(0x3, st = inf.GetBits(2));
        EXPECT_EQ(std::string(), st.error_text);
        EXPECT_EQ(0x0, st = inf.GetBits(2));
        EXPECT_EQ(std::string(), st.error_text);
        EXPECT_EQ(0x1, st = inf.GetBits(2));
        EXPECT_EQ(std::string(), st.error_text);

        EXPECT_EQ(0x0, st = inf.GetBits(1));
        EXPECT_EQ(true, ((bool)st));
        EXPECT_EQ(-5, inf.GetBits(42).error_code);

        EXPECT_EQ(-1, inf.FetchByte().error_code);
        EXPECT_EQ(-1, inf.FetchFull().error_code);
        inf.bit_buffer = -1;
        EXPECT_EQ(-2, inf.FetchByte().error_code);
        EXPECT_EQ(-2, inf.FetchFull().error_code);
    }
} // algorithm
} // util
} // trillek

#endif
