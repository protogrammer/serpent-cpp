#ifndef SERPENT_TABLES_HPP
#define SERPENT_TABLES_HPP

using PermutationTable = unsigned char[128];
using IndexTable = unsigned char[8][16];
using XorTable = unsigned char[128][8];


extern const PermutationTable IPTable;
extern const PermutationTable FPTable;

extern const IndexTable STable;
extern const IndexTable ITable;

extern const unsigned char End;
extern const XorTable LTTable;
extern const XorTable LTITable;

#endif  // SERPENT_TABLES_HPP
