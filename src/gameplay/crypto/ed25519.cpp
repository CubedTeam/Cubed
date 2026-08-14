#include "Cubed/crypto/ed25519.hpp"

#include <algorithm>
#include <sodium.h>
namespace Cubed::Crypto {
Ed25519KeyPair Ed25519::generate_key_pair() {
    Ed25519KeyPair pair;
    crypto_sign_keypair(pair.public_key.data.data(),
                        pair.private_key.data.data());

    return pair;
}

Uuid Ed25519::uuid_from_public_key(const Ed25519PublicKey& key) {
    std::array<unsigned char, crypto_hash_sha256_BYTES> hash{};
    crypto_hash_sha256(hash.data(), key.data.data(), key.data.size());

    std::array<std::uint8_t, 16> uuid{};
    std::copy_n(hash.begin(), uuid.size(), uuid.data());

    uuid[6] = static_cast<std::uint8_t>((uuid[6] & 0x0F) | 0x80);

    uuid[8] = static_cast<std::uint8_t>((uuid[8] & 0x3F) | 0x80);

    return Uuid{uuid};
}

Ed25519Signature Ed25519::sign(std::span<const unsigned char> challenge,
                               const Ed25519PrivateKey& private_key) {
    Ed25519Signature sign;
    crypto_sign_detached(sign.data.data(), nullptr, challenge.data(),
                         challenge.size(), private_key.data.data());

    return sign;
}

bool Ed25519::verify(std::span<const unsigned char> challenge,
                     const Ed25519Signature& signature,
                     const Ed25519PublicKey& public_key) {
    return crypto_sign_verify_detached(signature.data.data(), challenge.data(),
                                       challenge.size(),
                                       public_key.data.data()) == 0;
}

std::string Ed25519::to_hex(const unsigned char* data, std::size_t size) {
    std::string output(size * 2 + 1, '\0');
    sodium_bin2hex(output.data(), output.size(), data, size);
    output.resize(size * 2);
    return output;
}

bool Ed25519::from_hex(const std::string& hex, unsigned char* output,
                       std::size_t output_size) {
    std::size_t decoded_size = 0;

    return sodium_hex2bin(output, output_size, hex.data(), hex.size(), nullptr,
                          &decoded_size, nullptr) == 0 &&
           decoded_size == output_size;
}

} // namespace Cubed::Crypto