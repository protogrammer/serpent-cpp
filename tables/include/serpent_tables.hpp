#ifndef SERPENT_TABLES_HPP
#define SERPENT_TABLES_HPP

#include <cstddef>  // size_t

using PermutationTable = size_t[128];
using IndexTable = size_t[8][16];
using XorTable = size_t[128][8];


extern const PermutationTable IPTable;
extern const PermutationTable FPTable;

extern const IndexTable STable;
extern const IndexTable ITable;

extern const size_t End;
extern const XorTable LTTable;
extern const XorTable LTITable;

#endif  // SERPENT_TABLES_HPP
