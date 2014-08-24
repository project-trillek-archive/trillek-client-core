#ifndef ESIGN_SIGNATURE_H_INCLUDED
#define ESIGN_SIGNATURE_H_INCLUDED

#include <functional>
#include <memory>
#include "controllers/network/crypto.hpp"
#include "crypto++/esign.h"
#include "crypto++/tiger.h"
#include "crypto++/queue.h"

namespace trillek { namespace network { namespace cryptography {

using namespace std::placeholders;

class ESIGN_Signature : public std::enable_shared_from_this<ESIGN_Signature> {

public:
    ESIGN_Signature() {};

    virtual ~ESIGN_Signature() {};

    /** \brief Load the keys in the module
     *
     */
    void Initialize() {
        // TODO: load the keys from disk ?
        signer = ESIGN<Tiger>::Signer{privatekey};
        verifier = ESIGN<Tiger>::Verifier{publickey};
    }

    /** \brief Set the public key from a buffer. Needed by the verifier
     *
     * \param key std::unique_ptr<std::vector<byte>>&& the buffer containing the key to store
     * \return void
     *
     */
    void SetPublicKey(std::unique_ptr<std::vector<byte>>&& key) {
        ByteQueue bq;
        bq.Put(key->data(), key->size());
        publickey.Load(bq);
    }

    /** \brief Generate valid keys from scratch
     *
     * \return bool always true
     *
     */
    bool GenerateKeys() {
        InvertibleESIGNFunction parameters;

        // Modulus size should be a multiple of 3
        // k = 32 by default
        parameters.GenerateRandomWithKeySize( Crypto::RNG(), 64 * 3 );
        privatekey = ESIGN<Tiger>::PrivateKey{parameters};
        publickey = ESIGN<Tiger>::PublicKey{parameters};
        // TODO: save the keys to disk ?
        return true;
    };

    /** \brief Sign a message using an ESIGN signature
     *
     * \param sig byte* the buffer where the signature will be stored (24 bytes)
     * \param message const byte* the message to sign
     * \param len size_t the length of the message
     *
     */
    void Sign(byte* sig, const byte* message, size_t len) const {
        signer.SignMessage(Crypto::RNG(), message, len, sig);
    }

    /** \brief Verify a message with an ESIGN signature
     *
     * \param sig const byte* the signature (24 bytes)
     * \param message const byte* the message to verify
     * \param len size_t the length of the message
     * \return bool true if the signature is valid, false otherwise
     *
     */
    bool Verify(const byte* sig, const byte* message, size_t len) const {
        return verifier.VerifyMessage(message, len, sig, 24);
    }

    /** \brief Return the public key
     *
     * \return std::vector<byte> the public key
     *
     */
    std::vector<byte> PublicKey() const {
        ByteQueue queue;
        publickey.Save(queue);
        std::vector<byte> ret(queue.MaxRetrievable());
        queue.Get(ret.data(), ret.size());
        return ret;
    }

    /** \brief Return a functor that can sign messages
     *
     * \return std::function<void(unsigned char*,const unsigned char*,size_t)> the functor
     *
     */
    std::function<void(unsigned char*,const unsigned char*,size_t)> Hasher() const {
        return std::bind(&cryptography::ESIGN_Signature::Sign, this->shared_from_this(), _1, _2, _3);
    }

    /** \brief Return a functor that can verify messages
     *
     * \return std::function<bool(const unsigned char*,const unsigned char*,size_t)> the functor
     *
     */
    std::function<bool(const unsigned char*,const unsigned char*,size_t)> Verifier() const {
        return std::bind(&cryptography::ESIGN_Signature::Verify, this->shared_from_this(), _1, _2, _3);
    }

private:
    ESIGN<Tiger>::PrivateKey privatekey;
    ESIGN<Tiger>::PublicKey publickey;
    ESIGN<Tiger>::Signer signer;
    ESIGN<Tiger>::Verifier verifier;
};
} // cryptography
} // network
} // trillek

#endif // ESIGN_SIGNATURE_H_INCLUDED
