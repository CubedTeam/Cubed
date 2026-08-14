#pragma once
#include "Cubed/tools/uuid.hpp"

#include <array>
#include <span>
namespace Cubed::Crypto {
struct Ed25519PublicKey {
    std::array<unsigned char, 32> data;
};

struct Ed25519PrivateKey {
    std::array<unsigned char, 64> data;
};

struct Ed25519Signature {
    std::array<unsigned char, 64> data;
};

struct Ed25519KeyPair {
    Ed25519PublicKey public_key;
    Ed25519PrivateKey private_key;
};

class Ed25519 final {
public:
    static Ed25519KeyPair generate_key_pair();

    static Uuid uuid_from_public_key(const Ed25519PublicKey& key);

    static Ed25519Signature sign(std::span<const unsigned char> challenge,
                                 const Ed25519PrivateKey& private_key);

    static bool verify(std::span<const unsigned char> challenge,
                       const Ed25519Signature& signature,
                       const Ed25519PublicKey& public_key);

    static std::string to_hex(const unsigned char* data, std::size_t size);
    static bool from_hex(const std::string& hex, unsigned char* output,
                         std::size_t output_size);
};

} // namespace Cubed::Crypto