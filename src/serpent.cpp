#include "serpent.hpp"

#include "serpent_tables.hpp"
#include <bit>  // rotl, rotr
#include <algorithm> // copy_n, copy, fill
#include <iostream> // cout, endl, hex

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
    while (*indices != END)
        sum ^= getBit(block, *indices++);
    return sum;
}

static int S_iteration(const Block& block, size_t i, size_t j, const IndexTable& indexTable, const XorTable& xorTable) {
    int val = 0;
    for (size_t bitN = 0; bitN < SBoxSize; ++bitN) {
        val = (val << 1) | xorIndices(block, xorTable[j][bitN]);
    }
    std::cout << "SBox iteration(i=" << i << ", j=" << j << ", val=" << val << ", res=" << indexTable[i][val] << ")\n";
    return indexTable[i][val];
}

static void S(size_t i, const Block &block, Block& newBlock, const IndexTable& indexTable, const XorTable& xorTable) {
    for (size_t j = 0; j < BlockSize; ++j) {
        newBlock[j] = S_iteration(block, i, 2*j, indexTable, xorTable) 
                    | S_iteration(block, i, 2*j+1, indexTable, xorTable) << 4;
        std::cout << "New value: " << (int)newBlock[j] << std::endl;
    }
}


void applyPermutation(const Block& block, Block& newBlock, const PermutationTable& permutationTable) {
    for (size_t i = 0; i < BlockSize * 8; ++i)
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

static void keyShedule(const Serpent::Key& key, Block (&roundKeys)[Rounds + 1]) {
    uint32_t wBase[WordsInKey + WordsInBlock * (Rounds + 1)];
    std::copy_n(reinterpret_cast<const uint32_t*>(key), WordsInKey, wBase);
    uint32_t* w = wBase + WordsInKey;
    for (size_t i = 0; i < WordsInBlock * (Rounds + 1); ++i)
        w[i] = std::rotl(w[i - 8] ^ w[i - 5] ^ w[i - 3] ^ w[i - 1] ^ Phi ^ i, 11);
    for (size_t i = 0; i < Rounds + 1; ++i)
        S(7 - (i + 4) % 8, reinterpret_cast<const Block*>(w)[i], roundKeys[i], S_INDEX_TABLE, S_XOR_TABLE);
}


static void xorBlockInplace(Block& dst, const Block& src1) {
    for (size_t i = 0; i < BlockSize; ++i)
        dst[i] ^= src1[i];
}

static void xorBlock(Block& dst, const Block& src1, const Block& src2) {
    for (size_t i = 0; i < BlockSize; ++i)
        dst[i] = src1[i] ^ src2[i];
}

static void printBlock(const Block& block, const std::string& name) {
    std::cout << name << ':';
    for (auto x: block)
        std::cout << ' ' << (int)x;
    std::cout << std::endl;
}

void Serpent::encrypt(Block& block) const {
    Block subkeys[Rounds + 1];
    keyShedule(key, subkeys);

    std::cout << std::hex;
    printBlock(block, "Initial block");

    Block temp;
    applyPermutation(block, temp, IP_PERM_TABLE);
    printBlock(temp, "Block after IP");
    for (int i = 0; i < Rounds; ++i) {
        printBlock(subkeys[i], "Subkey #" + std::to_string(i + 1));
        xorBlock(block, temp, subkeys[i]);
        printBlock(block, "Block after xor");
        S(i % 8, block, temp, S_INDEX_TABLE, S_XOR_TABLE);
        printBlock(temp, "Block after S");
        linearTransformation(temp);
        printBlock(temp, "Block after LT");
    }
    printBlock(subkeys[Rounds], "Subkey #" + std::to_string(Rounds + 1));
    xorBlockInplace(temp, subkeys[Rounds]);
    printBlock(temp, "Block after xor");
    applyPermutation(temp, block, FP_PERM_TABLE);
    printBlock(block, "Final block");
}

void Serpent::decrypt(Block& block) const {
    Block subkeys[Rounds + 1];
    keyShedule(key, subkeys);

    Block temp;
    applyPermutation(block, temp, FP_PERM_TABLE);
    xorBlockInplace(temp, subkeys[32]);
    for (int i = 0; i < Rounds; ++i) {
        xorBlock(block, temp, subkeys[Rounds - i - 1]);
        S((Rounds - i - 1) % 8, block, temp, I_INDEX_TABLE, I_XOR_TABLE);
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
