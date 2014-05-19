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

    EXPECT_EQ(1, inf.LoadByte().error_code);
    EXPECT_EQ(1, inf.LoadFull().error_code);
    inf.bit_buffer = -1;
    EXPECT_EQ(-2, inf.LoadByte().error_code);
    EXPECT_EQ(-2, inf.LoadFull().error_code);
}

TEST(DecompressorTest, InflateSomething) {
    // zlib DEFLATE "compressed" string
    const unsigned char testdata[] = {
        0x78, 0x9c, 0x0b, 0xc9, 0x48, 0x55, 0x28, 0x49,
        0x2d, 0x2e, 0x51, 0x28, 0x2e, 0x29, 0xca, 0xcc,
        0x4b, 0x87, 0x51, 0x69, 0xa5, 0x39, 0x39, 0x0a,
        0xf9, 0x69, 0x0a, 0x29, 0x89, 0x25, 0x89, 0x0a,
        0x39, 0xf9, 0x68, 0x10, 0x00, 0x17, 0x1e, 0x14,
        0x56
    };
    // original string
    const unsigned char testresult[] =
        "The test string string full of data lolololololololol";
    DataString comp;
    DataString decomp;
    DataString result;
    comp.assign(testdata, sizeof(testdata));
    result.assign(testresult, sizeof(testresult) - 1); // don't need the c string null
    Inflate inf;
    EXPECT_EQ(false, inf.DecompressStart());
    EXPECT_EQ(false, inf.DecompressData(comp));
    EXPECT_EQ(false, inf.DecompressEnd());
    EXPECT_EQ(true, inf.DecompressHasOutput());
    decomp = inf.DecompressGetOutput();
    EXPECT_EQ(result.length(), decomp.length());
    EXPECT_EQ(result, decomp);
}

} // algorithm
} // util
} // trillek

#endif
