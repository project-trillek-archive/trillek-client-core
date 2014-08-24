#include "controllers/network/crypto.hpp"
//#include <iostream>
#include "crypto++/vmac.h"
#include "crypto++/sha.h"
#include "crypto++/pwdbased.h"

namespace trillek { namespace network {

/*  void Crypto::InitializeDH() {
		// http://tools.ietf.org/html/rfc5114#section-2.2
		Integer p("0xAD107E1E9123A9D0D660FAA79559C51FA20D64E5683B9FD1"
			"B54B1597B61D0A75E6FA141DF95A56DBAF9A3C407BA1DF15"
			"EB3D688A309C180E1DE6B85A1274A0A66D3F8152AD6AC212"
			"9037C9EDEFDA4DF8D91E8FEF55B7394B7AD5B7D0B6C12207"
			"C9F98D11ED34DBF6C6BA0B2C8BBC27BE6A00E0A0B9C49708"
			"B3BF8A317091883681286130BC8985DB1602E714415D9330"
			"278273C7DE31EFDC7310F7121FD5A07415987D9ADC0A486D"
			"CDF93ACC44328387315D75E198C641A480CD86A1B9E587E8"
			"BE60E69CC928B2B9C52172E413042E9B23F10B0E16E79763"
			"C9B53DCF4BA80A29E3FB73C16B8E75B97EF363E2FFA31F71"
			"CF9DE5384E71B81C0AC4DFFE0C10E64F");

		Integer g("0xAC4032EF4F2D9AE39DF30B5C8FFDAC506CDEBE7B89998CAF"
			"74866A08CFE4FFE3A6824A4E10B9A6F0DD921F01A70C4AFA"
			"AB739D7700C29F52C57DB17C620A8652BE5E9001A8D66AD7"
			"C17669101999024AF4D027275AC1348BB8A762D0521BC98A"
			"E247150422EA1ED409939D54DA7460CDB5F6C6B250717CBE"
			"F180EB34118E98D119529A45D6F834566E3025E316A330EF"
			"BB77A86F0C1AB15B051AE3D428C8F8ACB70A8137150B8EEB"
			"10E183EDD19963DDD9E263E4770589EF6AA21E7F5F2FF381"
			"B539CCE3409D13CD566AFBB48D6C019181E1BCFE94B30269"
			"EDFE72FE9B6AA4BD7B5A0F1C71CFFF4C19C418E1F6EC0179"
			"81BC087F2A7065B384B890D3191F2BFA");

		Integer q("0x801C0D34C58D93FE997177101F80535A4738CEBCBF389A99"
			"B36371EB");

		_dh.AccessGroupParameters().Initialize(p, q, g);

		_priv = SecByteBlock(_dh.PrivateKeyLength());
		_pub = SecByteBlock(_dh.PublicKeyLength());

		_dh.GenerateKeyPair(_prng, _priv, _pub);
	}

	void Crypto::GetPublicKeys(char* buffer) const {
		std::memcpy(buffer, _pub, _dh.PublicKeyLength());
	}

*/

AutoSeededRandomPool Crypto::_prng(false);
std::mutex Crypto::_prng_m;

void Crypto::GetRandom64(byte* nonce) {
    std::unique_lock<std::mutex> locker(_prng_m);
    _prng.GenerateBlock(nonce, 8);
};

void Crypto::GetRandom128(byte* nonce) {
    std::unique_lock<std::mutex> locker(_prng_m);
    _prng.GenerateBlock(nonce, 16);
};

void Crypto::VMAC64(byte* digest, const byte* message, size_t len, const byte* key, const byte* nonce) {
    CryptoPP::VMAC<CryptoPP::AES, 64> hasher;
    hasher.SetKey(key, 16, MakeParameters(Name::IV(), ConstByteArrayParameter(nonce, 8), false));
    hasher.CalculateDigest(digest, message, len);
/*    Integer vmac, nonc, k, m;
    vmac.Decode(digest, 8);
    nonc.Decode(nonce, 8);
    k.Decode(key, 16);
    m.Decode(message, len);
    std::cout << ">>DIGEST VMAC64" << std::endl;
    std::cout << "VMAC is " << std::hex << vmac << std::endl;
    std::cout << "nonce is " << std::hex << nonc << std::endl;
    std::cout << "key is " << std::hex << k << std::endl;
    std::cout << "message is " << std::hex << m << std::endl;*/
}

void Crypto::VMAC128(byte* digest, const byte* message, size_t len, const byte* key, const byte* nonce) {
    CryptoPP::VMAC<CryptoPP::AES, 128> hasher;
    hasher.SetKey(key, 16, MakeParameters(Name::IV(), ConstByteArrayParameter(nonce, 16), false));
    hasher.CalculateDigest(digest, message, len);
/*    Integer vmac, nonc, k, m;
    vmac.Decode(digest, 16);
    nonc.Decode(nonce, 16);
    k.Decode(key, 16);
    m.Decode(message, len);
    std::cout << ">>DIGEST VMAC128" << std::endl;
    std::cout << "VMAC is " << std::hex << vmac << std::endl;
    std::cout << "nonce is " << std::hex << nonc << std::endl;
    std::cout << "key is " << std::hex << k << std::endl;
    std::cout << "message is " << std::hex << m << std::endl;*/
}

bool Crypto::VMAC_Verify(const byte* digest, const byte* message, size_t len, const byte* key, const byte* nonce) {
    CryptoPP::VMAC<CryptoPP::AES, 64> hasher;
    hasher.SetKey(key, 16, MakeParameters(Name::IV(), ConstByteArrayParameter(nonce, 8), false));
/*    Integer vmac, nonc, k, m;
    vmac.Decode(digest, 8);
    nonc.Decode(nonce, 8);
    k.Decode(key, 16);
    m.Decode(message, len);
    std::cout << ">>Verify VMAC" << std::endl;
    std::cout << "VMAC is " << std::hex << vmac << std::endl;
    std::cout << "nonce is " << std::hex << nonc << std::endl;
    std::cout << "key is " << std::hex << k << std::endl;
    std::cout << "message is " << std::hex << m << std::endl;*/
    return hasher.VerifyDigest(digest, message, len);
}

void Crypto::PBKDF(byte* derived, const byte* password, size_t password_len, const byte* salt, size_t salt_len) {
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256>().DeriveKey(derived, 16, 0, password, password_len, salt, salt_len, 1024);
/*    Integer d, p, s;
    d.Decode(derived, 16);
    p.Decode(password, password_len);
    s.Decode(salt, salt_len);
    std::cout << ">>Derive password" << std::endl;
    std::cout << "derived key is " << std::hex << d << std::endl;
    std::cout << "password is " << std::hex << p << std::endl;
    std::cout << "salt is " << std::hex << s << std::endl;*/
}
} // network
} // trillek
