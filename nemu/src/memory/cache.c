#include "common.h"
#include "memory/cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "burst.h"

void InitCache();
int ReadCache_L1(hwaddr_t hwaddr);
void WriteCache_L1(hwaddr_t hwaddr, size_t Length, uint32_t data);

void ddr3_read_call(hwaddr_t addr, void *data);
void ddr3_write_call(hwaddr_t addr, void *data, uint8_t *mask);
void dram_write(hwaddr_t addr, size_t len, uint32_t data);

void InitCache(){
    int i;
    for(i = 0; i < CACHE_SIZE_L1/CACHE_BLOCK_SIZE_L1; ++i){
        Cache_L1[i].valid = 0;
    }
    clock_time = 0;
}

int ReadCache_L1(hwaddr_t hwaddr){
    uint32_t set_index = (hwaddr >> CACHE_BLOCK_BIT_L1) & (CACHE_SET_SIZE - 1);
    uint32_t tag = (hwaddr >> (CACHE_SET_BIT_L1 + CACHE_BLOCK_BIT_L1));
    uint32_t block_beginning = (hwaddr >> CACHE_BLOCK_BIT_L1) << CACHE_BLOCK_BIT_L1;

    int i, set = set_index * CACHE_WAY_SIZE_L1;
    for(i = set; i < set + CACHE_WAY_SIZE_L1; ++i){
        if(Cache_L1[i].valid == 1 && Cache_L1[i].tag == tag){
            /*read hit*/
            clock_time += 2;
            return i;
        }
    }

    /*hit missed, random substitution(PA3 task 1)*/
    srand(time(0));
    i = set + rand()%CACHE_WAY_SIZE_L1;
    int j;
    for(j = 0; j < CACHE_BLOCK_SIZE_L1/BURST_LEN; ++j){
        ddr3_read_call(block_beginning + BURST_LEN * j, Cache_L1[i].data + BURST_LEN * j);
    }
    clock_time += 200;//hit missed
    Cache_L1[i].valid = 1;
    Cache_L1[i].tag = tag;
    return i;
}

void WriteCache_L1(hwaddr_t hwaddr, size_t Length, uint32_t data){
    uint32_t set_index = (hwaddr >> CACHE_BLOCK_BIT_L1) & (CACHE_SET_SIZE - 1);
    uint32_t tag = (hwaddr >> (CACHE_SET_BIT_L1 + CACHE_BLOCK_BIT_L1));
    uint32_t offset = hwaddr & (CACHE_BLOCK_SIZE_L1 - 1);

    int i, set = set_index * CACHE_WAY_SIZE_L1;
    for(i = set; i < set + CACHE_WAY_SIZE_L1; ++i){
        if(Cache_L1[i].valid == 1 && Cache_L1[i].tag == tag){
            /*write hit, write through*/
            if(offset + Length > CACHE_BLOCK_SIZE_L1){
                dram_write(hwaddr, CACHE_BLOCK_SIZE_L1 - offset, data);
                memcpy(Cache_L1[i].data + offset, &data, CACHE_BLOCK_SIZE_L1 - offset);
                WriteCache_L1(hwaddr + CACHE_BLOCK_SIZE_L1 - offset, Length - (CACHE_BLOCK_SIZE_L1 - offset), data >> (CACHE_BLOCK_SIZE_L1 - offset));
            }
            else{
                dram_write(hwaddr, Length, data);
                memcpy(Cache_L1[i].data + offset, &data, Length);
            }
            return;
        }
    }
    /*not write allocate*/
    dram_write(hwaddr, Length, data);
}