#ifndef CACHE_H
#define CACHE_H
#include "common.h"


/*
L1:
block size 64 B
cache size 64 KB
8-way set associative
only valid bit
random substitution
write through
not write allocate

L2:
block size 64 B
cache size 4 MB
16-way set associative
valid bit and dirty bit
random substitution
write back
write allocate
*/

//L1
#define CACHE_BLOCK_SIZE_L1 64
#define CACHE_SIZE_L1 64*1024
#define CACHE_WAY_BIT_L1 3//8-way set associative, 3 bits
#define CACHE_BLOCK_BIT_L1 6//64 B, 6 bits
#define CACHE_SET_BIT_L1 7//64KB/(64B*8) = 2^7
#define CACHE_SET_SIZE (1 << CACHE_SET_BIT_L1)
#define CACHE_WAY_SIZE_L1 (1 << CACHE_WAY_BIT_L1)
#define Test

//L2
#define CACHE_BLOCK_SIZE_L2 64
#define CACHE_SIZE_L2 4*1024*1024
#define CACHE_WAY_BIT_L2 4//16-way set associative, 3 bits
#define CACHE_BLOCK_BIT_L2 6//64 B, 6 bits
#define CACHE_SET_BIT_L2 12//4MB/(64B*16) = 2^12
#define CACHE_SET_SIZE_L2 (1 << CACHE_SET_BIT_L2)
#define CACHE_WAY_SIZE_L2 (1 << CACHE_WAY_BIT_L2)

#ifdef Test
uint64_t clock_time;
#endif

//L1
typedef struct{
    uint8_t data[CACHE_BLOCK_SIZE_L1];
    uint32_t tag;
    bool valid;
}Cache_Block_L1;

Cache_Block_L1 Cache_L1[CACHE_SIZE_L1/CACHE_BLOCK_SIZE_L1];

//L2
typedef struct{
    uint8_t data[CACHE_BLOCK_SIZE_L2];
    uint32_t tag;
    bool valid;
    bool dirty;
}Cache_Block_L2;

Cache_Block_L2 Cache_L2[CACHE_SIZE_L2/CACHE_BLOCK_SIZE_L2];

void InitCache();
int ReadCache_L1(hwaddr_t addr);
void WriteCache_L1(hwaddr_t addr, size_t Length, uint32_t data);

int ReadCache_L2(hwaddr_t addr);
void WriteCache_L2(hwaddr_t addr, size_t Length, uint32_t data);

#endif