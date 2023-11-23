#ifndef SERPENT_TABLES_HPP
#define SERPENT_TABLES_HPP

#include <cstddef>  // size_t

using PermutationTable = size_t[128];
using IndexTable = size_t[8][16];
using XorTable = size_t[32][4][8];


extern const PermutationTable IP_PERM_TABLE;
extern const PermutationTable FP_PERM_TABLE;

extern const IndexTable S_INDEX_TABLE;
extern const IndexTable I_INDEX_TABLE;

extern const size_t END;
extern const XorTable S_XOR_TABLE;
extern const XorTable I_XOR_TABLE;

#endif  // SERPENT_TABLES_HPP
