#pragma once
#include "Cubed/tools/uuid.hpp"

#include <array>
#include <span>
#include <vector>
namespace Cubed::Crypto {
struct Ed25519PublicKey {
    std::array<unsigned char, 32> data;
    bool operator==(const Ed25519PublicKey&) const = default;
};

struct Ed25519PrivateKey {
    std::array<unsigned char, 64> data;
    bool operator==(const Ed25519PrivateKey&) const = default;
};

struct Ed25519Signature {
    std::array<unsigned char, 64> data;
    bool operator==(const Ed25519Signature&) const = default;
};

struct Ed25519KeyPair {
    Ed25519PublicKey public_key;
    Ed25519PrivateKey private_key;
};

class Ed25519 final {
public:
    using Challenge = std::array<std::uint8_t, 32>;

    static std::optional<Ed25519PublicKey>
    public_key_from_proto_bytes(const std::string& key);

    static Challenge generate_challenge();

    static Ed25519KeyPair generate_key_pair();

    static Uuid uuid_from_public_key(const Ed25519PublicKey& key);

    static Ed25519Signature sign(std::span<const std::uint8_t> message,
                                 const Ed25519PrivateKey& private_key);

    static bool verify(std::span<const std::uint8_t> message,
                       const Ed25519Signature& signature,
                       const Ed25519PublicKey& public_key);

    static std::string to_hex(const unsigned char* data, std::size_t size);
    static bool from_hex(const std::string& hex, unsigned char* output,
                         std::size_t output_size);

    static std::vector<unsigned char>
    make_login_signing_data(std::span<const unsigned char> challenge,
                            const Crypto::Ed25519PublicKey& public_key);
};

} // namespace Cubed::Crypto