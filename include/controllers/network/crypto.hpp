#ifndef CRYPTO_H_INCLUDED
#define CRYPTO_H_INCLUDED

#include "crypto++/osrng.h"
#include "crypto++/dh.h"
#include <mutex>

using namespace CryptoPP;

namespace CryptoPP {
template<class T, int> class VMAC;
}

namespace trillek { namespace network {

class Crypto {
public:
//		void InitializeDH();
//		void GetPublicKeys(char* buffer) const;
    /** \brief Calculate a VMAC digest with a size of 8 bytes
     *
     * \param digest byte* the digest buffer to fill (8 bytes)
     * \param message const byte* the message buffer
     * \param len size_t the length of the message
     * \param key const byte* the key (16 bytes)
     * \param nonce const byte* the nonce (8 bytes)
     *
     */
    static void VMAC64(byte* digest, const byte* message, size_t len,
                                    const byte* key, const byte* nonce);


    /** \brief Calculate a VMAC digest with a size of 16 bytes
     *
     * \param digest byte* the digest buffer to fill (16 bytes)
     * \param message const byte* the message buffer
     * \param len size_t the length of the message
     * \param key const byte* the key (16 bytes)
     * \param nonce const byte* the nonce (16 bytes)
     *
     */
    static void VMAC128(byte* digest, const byte* message, size_t len,
                                    const byte* key, const byte* nonce);

    /** \brief Verify a VMAC digest of 8 bytes
     *
     * \param digest const byte* the digest buffer to fill (8 bytes)
     * \param message const byte* the message buffer
     * \param len size_t the length of the message
     * \param key const byte* the key (16 bytes)
     * \param nonce const byte* the nonce (8 bytes)
     * \return bool true if the digest matches, false otherwise
     *
     */
    static bool VMAC_Verify(const byte* digest, const byte* message, size_t len,
                                            const byte* key, const byte* nonce);

    /** \brief Get a random number of 8 bytes
     *
     * \param nonce byte* the buffer to fill with the random number (8 bytes)
     *
     */
    static void GetRandom64(byte* nonce);

    /** \brief Get a random number of 16 bytes
     *
     * \param nonce byte* the buffer to fill with the random number (16 bytes)
     *
     */
    static void GetRandom128(byte* nonce);

    /** \brief Password-Based Key Derivation Function using PKCS5_PBKDF2_HMAC
     *
     * The key has a length of 16 bytes. The algorithm used is SHA-256.
     *
     * \param derived byte* the buffer for the derived key (16 bytes)
     * \param password const byte* the password to derive
     * \param password_len size_t the length of the password
     * \param salt const byte* the salt used in the derivation function
     * \param salt_len size_t the size of the salt
     *
     */
    static void PBKDF(byte* derived, const byte* password,
                    size_t password_len, const byte* salt, size_t salt_len);

    /** \brief A pseudo-random number generator provided by crypto++
     *
     * \return AutoSeededRandomPool& the PRNG
     *
     */
    static AutoSeededRandomPool& RNG() { return _prng; };

private:

/*		DH _dh;
    SecByteBlock _priv;
    SecByteBlock _pub;

    // Random number generator*/
    static AutoSeededRandomPool _prng;
    // mutex to guard the PRNG
    // TODO: make a prng local to each thread and suppress synchronization, needs thread_local support
    static std::mutex _prng_m;
};
} // network
} // trillek

#endif // CRYPTO_H_INCLUDED
