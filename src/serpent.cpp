#include "serpent.hpp"

#include <bit>  // rotl
#include <algorithm>  // copy_n, copy, fill

#include "serpent_tables.hpp"

using Block = Serpent::Block;

static const uint32_t Phi = 0x9e3779b9;
static const size_t Rounds = 32;
static const size_t SBoxSize = 4;
static const size_t BlockSize = sizeof(Block);
static const size_t KeySize = sizeof(Serpent::Key);
static const size_t WordsInBlock = BlockSize / sizeof(uint32_t);
static const size_t WordsInKey = KeySize / sizeof(uint32_t);

static int getBit(const Block& block, size_t i) {
    return block[i / 8] & (1 << i % 8) ? 1 : 0;
}

static void setBit(Block& block, size_t i, int value) {
    if (value) block[i / 8] |= 1 << i % 8;
    else block[i / 8] &= ~(1 << i % 8);
}

static int xorIndices(const Block& block, const size_t* indices) {
    int sum = 0;
    while (*indices != End)
        sum ^= getBit(block, *indices++);
    return sum;
}

static void linearTransformation(const Block& src, Block& dst, const XorTable& table) {
    for (size_t i = 0; i < BlockSize*8; ++i)
        setBit(dst, i, xorIndices(src, table[i]));
}

static void sBox(size_t i, const Block &block, Block& newBlock, const IndexTable& table) {
    for (size_t j = 0; j < BlockSize; ++j)
        newBlock[j] = table[i][block[j] & 0b1111]
                    | table[i][block[j] >> 4] << 4;
}


static void applyPermutation(const Block& src, Block& dst, const PermutationTable& table) {
    for (size_t i = 0; i < BlockSize * 8; ++i)
        setBit(dst, i, getBit(src, table[i]));
}

static void applyPermutationInverse(const Block& src, Block& dst, const PermutationTable& table) {
    for (size_t i = 0; i < BlockSize * 8; ++i)
        setBit(dst, table[i], getBit(src, i));
}

static void keyShedule(const Serpent::Key& key, Block (&roundKeys)[Rounds + 1]) {
    uint32_t wBase[WordsInKey + WordsInBlock * (Rounds + 1)];
    std::copy_n(reinterpret_cast<const uint32_t*>(key), WordsInKey, wBase);
    uint32_t* w = wBase + WordsInKey;
    for (size_t i = 0; i < WordsInBlock * (Rounds + 1); ++i)
        w[i] = std::rotl(w[i - 8] ^ w[i - 5] ^ w[i - 3] ^ w[i - 1] ^ Phi ^ i, 11);
    for (size_t i = 0; i < Rounds + 1; ++i)
        sBox(7 - (i + 4) % 8, reinterpret_cast<const Block*>(w)[i], roundKeys[i], STable);
}


static void xorBlockInplace(Block& dst, const Block& src) {
    for (size_t i = 0; i < BlockSize; ++i)
        dst[i] ^= src[i];
}

void Serpent::encrypt(Block& block) const {
    Block subkeys[Rounds + 1];
    keyShedule(key, subkeys);

    Block temp;
    applyPermutation(block, temp, IPTable);
    for (int i = 0; i < Rounds; ++i) {
        xorBlockInplace(temp, subkeys[i]);
        sBox(i % 8, temp, block, STable);
        linearTransformation(block, temp, LTTable);
    }
    xorBlockInplace(temp, subkeys[Rounds]);
    applyPermutation(temp, block, FPTable);
}

void Serpent::decrypt(Block& block) const {
    Block subkeys[Rounds + 1];
    keyShedule(key, subkeys);

    Block temp;
    applyPermutationInverse(block, temp, FPTable);
    xorBlockInplace(temp, subkeys[Rounds]);
    for (int i = 0; i < Rounds; ++i) {
        linearTransformation(temp, block, LTITable);
        sBox((Rounds - i - 1) % 8, block, temp, ITable);
        xorBlockInplace(temp, subkeys[Rounds - i - 1]);
    }
    applyPermutationInverse(temp, block, IPTable);
}

Serpent::Serpent(const Key256& key) {
    std::copy(std::begin(key), std::end(key), std::begin(this->key));
}

Serpent::Serpent(const Key192& key) {
    std::copy(std::begin(key), std::end(key), std::begin(this->key));
    this->key[192 / 8] = 0xf0; // ???
    std::fill(std::begin(this->key) + 192 / 8 + 1, std::end(this->key), 0);
}

Serpent::Serpent(const Key128& key) {
    std::copy(std::begin(key), std::end(key), std::begin(this->key));
    this->key[128 / 8] = 0xf0; // ???
    std::fill(std::begin(this->key) + 128 / 8 + 1, std::end(this->key), 0);
}
