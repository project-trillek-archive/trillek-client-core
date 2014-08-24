#ifndef ESIGN_SIGNATURETEST_H_INCLUDED
#define ESIGN_SIGNATURETEST_H_INCLUDED

#include "controllers/network/ESIGN-signature.hpp"
#include "gtest/gtest.h"

using trillek::network::cryptography::ESIGN_Signature;

namespace trillek {

TEST(ESIGN_SignatureTest, BadSignature) {
    ESIGN_Signature esign;
    ASSERT_TRUE(esign.GenerateKeys()) << "Could not generate keys";
    esign.Initialize();
    std::vector<byte> tag(24);
    auto m = std::string("Not signed");
    ASSERT_FALSE(esign.Verify(tag.data(),  reinterpret_cast<const byte*>(m.data()), m.size())) << "Verify should return false";
}

TEST(ESIGN_SignatureTest, GoodSignature) {
    ESIGN_Signature esign;
    ASSERT_TRUE(esign.GenerateKeys()) << "Could not generate keys";
    esign.Initialize();
    std::vector<byte> tag(24);
    auto m = std::string("Sign me !");
    esign.Sign(tag.data(), reinterpret_cast<const byte*>(m.data()), m.size());
    ASSERT_TRUE(esign.Verify(tag.data(),  reinterpret_cast<const byte*>(m.data()), m.size())) << "Verify should return true";
}

TEST(ESIGN_SignatureTest, KeyManipulation) {
    ESIGN_Signature esign;
    ASSERT_TRUE(esign.GenerateKeys()) << "Could not generate keys";
    auto pubkey = make_unique<std::vector<unsigned char>>(esign.PublicKey());
    ASSERT_NE(pubkey, nullptr) << "PublicKey returned an empty smart pointer";
    ASSERT_EQ(pubkey->size(), 32) << "PublicKey unexpected length, should be 32";
    esign.SetPublicKey(std::move(pubkey));
    esign.Initialize();
    std::vector<byte> tag(24);
    auto m = std::string("Sign me !");
    esign.Sign(tag.data(), reinterpret_cast<const byte*>(m.data()), m.size());
    ASSERT_TRUE(esign.Verify(tag.data(),  reinterpret_cast<const byte*>(m.data()), m.size())) << "Verify should return true";
}
}
#endif // ESIGN_SIGNATURETEST_H_INCLUDED
