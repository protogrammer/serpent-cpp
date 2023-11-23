#include "serpent.hpp"

#include "serpent_tables.hpp"
#include <bit>  // rotl, rotr
#include <algorithm> // copy_n, copy, fill

using Block = Serpent::Block;

static int getBit(const Block& block, size_t i) {
    return block[i / 8] & (1 << i % 8) ? 1 : 0;
}

static void setBit(Block& block, size_t i, int value) {
    if (value) block[i / 8] |= 1 << i % 8;
    else block[i / 8] &= ~(1 << i % 8);
}


static int xorIndices(const Block& block, const size_t* indices) {
    int sum = 0;
    while (*indices != END)
        sum ^= getBit(block, *indices++);
    return sum;
}

static int S_iteration(const Block& block, size_t i, size_t j, const IndexTable& indexTable, const XorTable& xorTable) {
    int val = 0;
    for (size_t bitN = 0; bitN < 4; ++bitN) // magic SBOX_BIT_SIZE
        val = (val >> 1) | xorIndices(block, xorTable[j][bitN]);
    return indexTable[i][val];
}

static void S(size_t i, const Block &block, Block& newBlock, const IndexTable& indexTable, const XorTable& xorTable) {
    for (size_t j = 0; j < sizeof block; ++j)  // magic BLOCK_SIZE
        newBlock[j] = S_iteration(block, i, 2*j, indexTable, xorTable) 
                    | S_iteration(block, i, 2*j+1, indexTable, xorTable) << 4;
}


void applyPermutation(const Block& block, Block& newBlock, const PermutationTable& permutationTable) {
    for (size_t i = 0; i < sizeof newBlock * 8; ++i)  // magic BLOCK_SIZE
        setBit(newBlock, i, getBit(block, permutationTable[i]));
}


static void linearTransformation(Block& block) {
    auto& X0 = *reinterpret_cast<uint32_t*>(block);
    auto& X1 = (&X0)[1];
    auto& X2 = (&X0)[2];
    auto& X3 = (&X0)[3];

    X0 = std::rotl(X0, 13);
    X2 = std::rotl(X2, 3);
    X1 = X1 ^ X0 ^ X2;
    X3 = X3 ^ X2 ^ (X0 << 3);
    X1 = std::rotl(X1, 1);
    X3 = std::rotl(X3, 7);
    X0 = X0 ^ X1 ^ X3;
    X2 = X2 ^ X3 ^ (X1 << 7);
    X0 = std::rotl(X0, 5);
    X2 = std::rotl(X2, 22);
}

static void linearTransformationInverse(Block& block) {
    auto& X0 = *reinterpret_cast<uint32_t*>(block);
    auto& X1 = (&X0)[1];
    auto& X2 = (&X0)[2];
    auto& X3 = (&X0)[3];

    X2 = std::rotr(X2, 22);
    X0 = std::rotr(X0, 5);
    X2 = X2 ^ X3 ^ (X1 << 7);
    X0 = X0 ^ X1 ^ X3;
    X3 = std::rotr(X3, 7);
    X1 = std::rotr(X1, 1);
    X3 = X3 ^ X2 ^ (X0 << 3);
    X1 = X1 ^ X0 ^ X2;
    X2 = std::rotr(X2, 3);
    X0 = std::rotr(X0, 13);
}

// TODO get rid of "magic numbers"

static const uint32_t PHI = 0x9e3779b9;
static const size_t ROUNDS = 32;

static void keyShedule(const Serpent::Key& key, Block (&roundKeys)[ROUNDS + 1]) {
    uint32_t wBase[4 * (ROUNDS + 1) + 8];  // magic SBOX_BIT_SIZE
    std::copy_n(reinterpret_cast<const uint32_t*>(key), 8, wBase); // magic
    uint32_t* w = wBase + 8; // magic
    for (size_t i = 0; i < 4 * (ROUNDS + 1); ++i)
        w[i] = std::rotl(w[i - 8] ^ w[i - 5] ^ w[i - 3] ^ w[i - 1] ^ PHI ^ i, 11);
    for (size_t i = 0; i < ROUNDS + 1; ++i)
        S(7 - (i + 4) % 8, reinterpret_cast<const Block*>(w)[i], roundKeys[i], S_INDEX_TABLE, S_XOR_TABLE);
}


static void xorBlockInplace(Block& dst, const Block& src1) {
    for (size_t i = 0; i < 16; ++i) // magic
        dst[i] ^= src1[i];
}

static void xorBlock(Block& dst, const Block& src1, const Block& src2) {
    for (size_t i = 0; i < 16; ++i) // magic
        dst[i] = src1[i] ^ src2[i];
}

void Serpent::encrypt(Block& block) const {
    Block subkeys[ROUNDS + 1];
    keyShedule(key, subkeys);

    Block temp;
    applyPermutation(block, temp, IP_PERM_TABLE);
    for (int i = 0; i < ROUNDS; ++i) {
        xorBlock(block, temp, subkeys[i]);
        S(i % 8, block, temp, S_INDEX_TABLE, S_XOR_TABLE);
        linearTransformation(temp);
    }
    xorBlockInplace(temp, subkeys[32]);
    applyPermutation(temp, block, FP_PERM_TABLE);
}

void Serpent::decrypt(Block& block) const {
    Block subkeys[ROUNDS + 1];
    keyShedule(key, subkeys);

    Block temp;
    applyPermutation(block, temp, FP_PERM_TABLE);
    xorBlockInplace(temp, subkeys[32]);
    for (int i = 0; i < ROUNDS; ++i) {
        xorBlock(block, temp, subkeys[ROUNDS - i - 1]);
        S((ROUNDS - i - 1) % 8, block, temp, I_INDEX_TABLE, I_XOR_TABLE);
        linearTransformation(temp);
    }
    applyPermutation(temp, block, IP_PERM_TABLE);
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
