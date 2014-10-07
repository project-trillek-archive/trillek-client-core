#ifndef VMAC_STREAMHASHERTEST_H_INCLUDED
#define VMAC_STREAMHASHERTEST_H_INCLUDED

#include "controllers/network/VMAC-stream-hasher.hpp"
#include <cstring>
#include "gtest/gtest.h"

using trillek::network::cryptography::VMAC_StreamHasher;

class VMAC_StreamHasherTest: public ::testing::Test {
public:
    VMAC_StreamHasherTest() : tag(8), m("abc") {
        rfc_tag = {
            "\x25\x76\xBE\x1C\x56\xD8\xB8\x1B",
            "\x2D\x37\x6C\xF5\xB1\x81\x3C\xE5",
            "\xE8\x42\x1F\x61\xD5\x73\xD2\x98",
            "\x44\x92\xDF\x6C\x5C\xAC\x1B\xBE",
            "\x09\xBA\x59\x7D\xD7\x60\x11\x13" };
    }

    void SetUp() {
        std::vector<byte> nonce(8);
        std::memcpy(nonce.data(), std::string("bcdefghh").data(), 8);

        auto key1 = new CryptoPP::FixedSizeAlignedSecBlock<byte,16>();
        std::memcpy(key1->data(), std::string("abcdefghijklmnop").data(), 16);
        vmac1 = std::make_shared<VMAC_StreamHasher>(VMAC_StreamHasher::buffer(key1), nonce.data(), 8);

        auto key2 = new CryptoPP::FixedSizeAlignedSecBlock<byte,16>();
        std::memcpy(key2->data(), std::string("abcdefghijklmnop").data(), 16);
        vmac2 = std::make_shared<VMAC_StreamHasher>(VMAC_StreamHasher::buffer(key2), nonce.data(), 8);
    }

    void multiply(std::string& s, unsigned int n) {
        auto ret = s;
        for (auto i = 0; i < n; ++i) {
            ret.append(s);
        }
        s = std::move(ret);
    }

protected:
    std::string m;
    std::vector<std::string> rfc_tag;
    std::shared_ptr<trillek::network::cryptography::VMAC_StreamHasher> vmac1;
    std::shared_ptr<trillek::network::cryptography::VMAC_StreamHasher> vmac2;
    std::vector<byte> tag;
};


namespace trillek {

TEST_F(VMAC_StreamHasherTest, BadMessage) {
    auto hasher = vmac1->Hasher();
    auto verifier = vmac2->Verifier();
    hasher(tag.data(), reinterpret_cast<const byte*>(m.data()), m.size());
    std::string s("bad message");
    ASSERT_FALSE(verifier(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size())) << "The tag was wrongly done as good";
}

TEST_F(VMAC_StreamHasherTest, TestVector0) {
    auto hasher = vmac1->Hasher();
    auto verifier = vmac2->Verifier();
    hasher(tag.data(), reinterpret_cast<const byte*>(m.data()), 0);
    ASSERT_EQ(std::memcmp(tag.data(), rfc_tag[0].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(verifier(tag.data(), reinterpret_cast<const byte*>(m.data()), 0)) << "Can not verify the tag for message " << m;
}

TEST_F(VMAC_StreamHasherTest, TestVector1) {
    auto hasher = vmac1->Hasher();
    auto verifier = vmac2->Verifier();
    hasher(tag.data(), reinterpret_cast<const byte*>(m.data()), m.size());
    ASSERT_EQ(std::memcmp(tag.data(), rfc_tag[1].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(verifier(tag.data(), reinterpret_cast<const byte*>(m.data()), m.size())) << "Can not verify the tag for message " << m;
}

TEST_F(VMAC_StreamHasherTest, TestVector2) {
    auto hasher = vmac1->Hasher();
    auto verifier = vmac2->Verifier();
    auto s = m;
    multiply(s, 15);
    hasher(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size());
    ASSERT_EQ(std::memcmp(tag.data(), rfc_tag[2].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(verifier(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size())) << "Can not verify the tag for message " << m;
}

TEST_F(VMAC_StreamHasherTest, TestVector3) {
    auto hasher = vmac1->Hasher();
    auto verifier = vmac2->Verifier();
    auto s = m;
    multiply(s, 99);
    hasher(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size());
    ASSERT_EQ(std::memcmp(tag.data(), rfc_tag[3].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(verifier(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size())) << "Can not verify the tag for message " << m;
}

TEST_F(VMAC_StreamHasherTest, TestVector4) {
    auto hasher = vmac1->Hasher();
    auto verifier = vmac2->Verifier();
    auto s = m;
    multiply(s, 999999);
    hasher(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size());
    ASSERT_EQ(std::memcmp(tag.data(), rfc_tag[4].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(verifier(tag.data(), reinterpret_cast<const byte*>(s.data()), s.size())) << "Can not verify the tag for message " << m;
}
}
#endif // VMAC_STREAMHASHERTEST_H_INCLUDED
