#ifndef SERPENT_HPP
#define SERPENT_HPP

#include <endian.h>  // __BYTE_ORDER, __LITTLE_ENDIAN

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "Serpent is implemented only for little-endian systems"
#endif

class Serpent final {
public:
    using Key256 = unsigned char[256/8];
    using Key192 = unsigned char[192/8];
    using Key128 = unsigned char[128/8];
    using Key = Key256;

    using Block = unsigned char[128/8];

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
