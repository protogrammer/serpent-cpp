#ifndef SERPENT_HPP
#define SERPENT_HPP

#include <endian.h>  // __BYTE_ORDER, __LITTLE_ENDIAN

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "Serpent is implemented only for little-endian systems"
#endif

#include <cstdint> // uint8_t

class Serpent final {
public:
    using Key256 = uint8_t[256/8];
    using Key192 = uint8_t[192/8];
    using Key128 = uint8_t[128/8];
    using Key = Key256;

    using Block = uint8_t[128/8];

private:
    Key key;

public:
    void encrypt(Block& block) const;
    void decrypt(Block& block) const;

    Serpent(const Key256& key);
    Serpent(const Key192& key);
    Serpent(const Key128& key);
};

#endif  // SERPENT_HPP
