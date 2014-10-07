#ifndef CRYPTOTEST_H_INCLUDED
#define CRYPTOTEST_H_INCLUDED

#include "controllers/network/crypto.hpp"
#include "gtest/gtest.h"

class CryptoTest: public ::testing::Test {
public:
    CryptoTest() : tag8(8), tag16(16), nonce(16), key(16), m("abc") {
        rfc_tag16 = {
            "\x1b\x33\xee\x8c\x2c\x02\xcd\x6b\xee\xd0\x0a\x42\x97\xca\x78\x96",
            "\x22\xf4\x9d\x65\x86\xab\x52\x35\x7a\x22\x64\x22\x03\xfa\xf5\x2c",
            "\xdd\xff\x4f\xd1\xaa\x9d\xe7\xe8\x67\xe2\x4a\xde\xb1\x27\x6f\x98",
            "\x3a\x50\x0f\xdc\x31\xd6\x31\x0e\xc4\x3c\xdd\xe6\x17\x62\xa6\xb5",
            "\xff\x77\x89\xed\xac\x8a\x26\x63\x1b\xfa\xfa\xad\x46\xa8\x94\xc6"
        };
        rfc_tag8 = {
            "\x25\x76\xBE\x1C\x56\xD8\xB8\x1B",
            "\x2D\x37\x6C\xF5\xB1\x81\x3C\xE5",
            "\xE8\x42\x1F\x61\xD5\x73\xD2\x98",
            "\x44\x92\xDF\x6C\x5C\xAC\x1B\xBE",
            "\x09\xBA\x59\x7D\xD7\x60\x11\x13"
        };
    }

    void SetUp() {
        std::memcpy(nonce.data(), std::string("bcdefghijklmnopq").data(), 16);
        std::memcpy(key.data(), std::string("abcdefghijklmnop").data(), 16);
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
    std::vector<std::string> rfc_tag16;
    std::vector<std::string> rfc_tag8;
    std::vector<byte> tag8;
    std::vector<byte> tag16;
    std::vector<byte> nonce;
    std::vector<byte> key;
};

namespace trillek { namespace network {
TEST_F(CryptoTest, BadMessage64) {
    Crypto::VMAC64(tag8.data(), reinterpret_cast<const byte*>(m.data()), m.size(), key.data(), nonce.data());
    std::string s("bad message");
    ASSERT_FALSE(Crypto::VMAC_Verify(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data())) << "The tag was wrongly done as good";
}

TEST_F(CryptoTest, Test64Vector0) {
    Crypto::VMAC64(tag8.data(), reinterpret_cast<const byte*>(m.data()), 0, key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag8.data(), rfc_tag8[0].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(Crypto::VMAC_Verify(tag8.data(), reinterpret_cast<const byte*>(m.data()), 0, key.data(), nonce.data())) << "Can not verify the tag for message " << m;
}

TEST_F(CryptoTest, Test64Vector1) {
    Crypto::VMAC64(tag8.data(), reinterpret_cast<const byte*>(m.data()), m.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag8.data(), rfc_tag8[1].data(), 8), 0) << "The tag for message " << m << " is wrong";
    ASSERT_TRUE(Crypto::VMAC_Verify(tag8.data(), reinterpret_cast<const byte*>(m.data()), m.size(), key.data(), nonce.data())) << "Can not verify the tag for message " << m;
}

TEST_F(CryptoTest, Test64Vector2) {
    auto s = m;
    multiply(s, 15);
    Crypto::VMAC64(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag8.data(), rfc_tag8[2].data(), 8), 0) << "The tag for message " << s << " is wrong";
    ASSERT_TRUE(Crypto::VMAC_Verify(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data())) << "Can not verify the tag for message " << s;
}

TEST_F(CryptoTest, Test64Vector3) {
    auto s = m;
    multiply(s, 99);
    Crypto::VMAC64(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag8.data(), rfc_tag8[3].data(), 8), 0) << "The tag for message " << s << " is wrong";
    ASSERT_TRUE(Crypto::VMAC_Verify(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data())) << "Can not verify the tag for message " << s;
}

TEST_F(CryptoTest, Test64Vector4) {
    auto s = m;
    multiply(s, 999999);
    Crypto::VMAC64(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag8.data(), rfc_tag8[4].data(), 8), 0) << "The tag for message " << s << " is wrong";
    ASSERT_TRUE(Crypto::VMAC_Verify(tag8.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data())) << "Can not verify the tag for message " << s;
}

TEST_F(CryptoTest, Test128Vector0) {
    Crypto::VMAC128(tag16.data(), reinterpret_cast<const byte*>(m.data()), 0, key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag16.data(), rfc_tag16[0].data(), 16), 0) << "The tag for message " << m << " is wrong";
}

TEST_F(CryptoTest, Test128Vector1) {
    Crypto::VMAC128(tag16.data(), reinterpret_cast<const byte*>(m.data()), m.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag16.data(), rfc_tag16[1].data(), 16), 0) << "The tag for message " << m << " is wrong";
}

TEST_F(CryptoTest, Test128Vector2) {
    auto s = m;
    multiply(s, 15);
    Crypto::VMAC128(tag16.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag16.data(), rfc_tag16[2].data(), 16), 0) << "The tag for message " << s << " is wrong";
}

TEST_F(CryptoTest, Test128Vector3) {
    auto s = m;
    multiply(s, 99);
    Crypto::VMAC128(tag16.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag16.data(), rfc_tag16[3].data(), 16), 0) << "The tag for message " << s << " is wrong";
}

TEST_F(CryptoTest, Test128Vector4) {
    auto s = m;
    multiply(s, 999999);
    Crypto::VMAC128(tag16.data(), reinterpret_cast<const byte*>(s.data()), s.size(), key.data(), nonce.data());
    ASSERT_EQ(std::memcmp(tag16.data(), rfc_tag16[4].data(), 16), 0) << "The tag for message " << s << " is wrong";
}

TEST_F(CryptoTest, PBKDF) {
    std::vector<byte> derived(16);
    std::string expected("\x23\x1a\xfb\x7d\xcd\x2e\x86\x0c\xfd\x58\xab\x13\x37\x2b\xd1\x2c");
    Crypto::PBKDF(derived.data(), reinterpret_cast<const byte*>("password"), 8,
                                                                reinterpret_cast<const byte*>("salt"), 4);
    ASSERT_EQ(std::memcmp(derived.data(), expected.data(), 16), 0) << "The derived key is wrong";
}
}
}
#endif // CRYPTOTEST_H_INCLUDED
