#include "common.h"
#include "memory/cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "burst.h"

void ddr3_read_call(hwaddr_t addr, void *data);
void ddr3_write_call(hwaddr_t addr, void *data, uint8_t *mask);
void dram_write(hwaddr_t addr, size_t len, uint32_t data);

void InitCache(){
    int i;
    for(i = 0; i < CACHE_SIZE_L1/CACHE_BLOCK_SIZE_L1; ++i){
        Cache_L1[i].valid = false;
    }
    for (i = 0;i < CACHE_SIZE_L2 / CACHE_BLOCK_SIZE_L2; ++i){
        Cache_L2[i].valid = false;
        Cache_L2[i].dirty = false;
    }
    clock_time = 0;
}

int ReadCache_L1(hwaddr_t hwaddr){
    uint32_t set_index = (hwaddr >> CACHE_BLOCK_BIT_L1) & (CACHE_SET_SIZE - 1);
    uint32_t tag = (hwaddr >> (CACHE_SET_BIT_L1 + CACHE_BLOCK_BIT_L1));
    //uint32_t block_beginning = (hwaddr >> CACHE_BLOCK_BIT_L1) << CACHE_BLOCK_BIT_L1;

    int i, set = set_index * CACHE_WAY_SIZE_L1;
    for(i = set; i < set + CACHE_WAY_SIZE_L1; ++i){
        if(Cache_L1[i].valid == 1 && Cache_L1[i].tag == tag){
            /*read hit*/
            clock_time += 2;
            return i;
        }
    }

    /*hit missed, random substitution(PA3 task 1)*/
    /*srand(time(0));
    i = set + rand()%CACHE_WAY_SIZE_L1;
    int j;
    for(j = 0; j < CACHE_BLOCK_SIZE_L1/BURST_LEN; ++j){
        ddr3_read_call(block_beginning + BURST_LEN * j, Cache_L1[i].data + BURST_LEN * j);
    }
    clock_time += 200;*///hit missed

    // get content in Cache2(optional task 1)
    int pl = ReadCache_L2(hwaddr);
    srand(time(0));
    i = set + rand() % CACHE_WAY_SIZE_L1;
    memcpy(Cache_L1[i].data,Cache_L2[pl].data, CACHE_BLOCK_SIZE_L1);

    Cache_L1[i].valid = true;
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
                /*Update Cache2*/
                WriteCache_L2(hwaddr, CACHE_BLOCK_SIZE_L1 - offset, data);
            }
            else{
                dram_write(hwaddr, Length, data);
                memcpy(Cache_L1[i].data + offset, &data, Length);
                /*Update Cache2*/
                WriteCache_L2(hwaddr,Length, data);
            }
            return;
        }
    }
    /*not write allocate(task 1)*/
    //dram_write(hwaddr, Length, data);

    //optional task1
    WriteCache_L2(hwaddr, Length, data);
}

int ReadCache_L2(hwaddr_t addr){
    uint32_t set_idx = (addr >> CACHE_BLOCK_BIT_L2) & (CACHE_SET_SIZE_L2 - 1);
    uint32_t tag = (addr >> (CACHE_SET_BIT_L2 + CACHE_BLOCK_BIT_L2));
    uint32_t block_start = (addr >> CACHE_BLOCK_BIT_L2) << CACHE_BLOCK_BIT_L2;

    int i,set = set_idx * CACHE_WAY_SIZE_L2;
    for (i = set + 0; i < set + CACHE_WAY_SIZE_L2; i++){
        if (Cache_L2[i].valid == 1 && Cache_L2[i].tag == tag){
            //read hit
            clock_time += 10;//Hit in Cache2
            return i;
        }
    }
    // Random (PA3 optional task1)
    clock_time += 200;//Hit missed in Cache2
    srand(time(0));
    i = set + rand() % CACHE_WAY_SIZE_L2;
    /*write back*/
    if (Cache_L2[i].valid == 1 && Cache_L2[i].dirty == 1){
        uint8_t ret[BURST_LEN << 1];
        uint32_t block_st = (Cache_L2[i].tag << (CACHE_SET_BIT_L2 + CACHE_BLOCK_BIT_L2)) | (set_idx << CACHE_BLOCK_BIT_L2);
        int w;
        memset(ret,1,sizeof ret);
        for (w = 0;w < CACHE_BLOCK_SIZE_L2 / BURST_LEN; w++){
            ddr3_write_call(block_st + BURST_LEN * w, Cache_L2[i].data + BURST_LEN * w,ret);
        }
    }
    /*new content*/
    int j;
    for (j = 0;j < CACHE_BLOCK_SIZE_L2 / BURST_LEN;j ++){
        ddr3_read_call(block_start + BURST_LEN * j, Cache_L2[i].data + BURST_LEN * j);
    }
    Cache_L2[i].dirty = false;
    Cache_L2[i].valid = true;
    Cache_L2[i].tag = tag;
    return i;
}

void WriteCache_L2(hwaddr_t addr, size_t len, uint32_t data){
    uint32_t set_idx = (addr >> CACHE_BLOCK_BIT_L2) & (CACHE_SET_SIZE_L2 - 1);
    uint32_t tag = (addr >> (CACHE_SET_BIT_L2 + CACHE_BLOCK_BIT_L2));
    uint32_t offset = addr & (CACHE_BLOCK_SIZE_L2 - 1);

    int i,group = set_idx * CACHE_WAY_SIZE_L2;
    for (i = group + 0; i < group + CACHE_WAY_SIZE_L2; i ++){
        if (Cache_L2[i].valid == 1 && Cache_L2[i].tag == tag){
            // WRITE HIT
            Cache_L2[i].dirty = 1;
            if (offset + len > CACHE_BLOCK_SIZE_L2){
                memcpy(Cache_L2[i].data + offset, &data, CACHE_BLOCK_SIZE_L2 - offset);
                WriteCache_L2(addr + CACHE_BLOCK_SIZE_L2 - offset,len - (CACHE_BLOCK_SIZE_L2 - offset),data >> (CACHE_BLOCK_SIZE_L2 - offset));
            }else {
                memcpy(Cache_L2[i].data + offset, &data, len);
            }
            return;
        }
    }
    /*write allocate*/
    i = ReadCache_L2(addr);
    Cache_L2[i].dirty = true;
    memcpy(Cache_L2[i].data + offset, &data, len);
}