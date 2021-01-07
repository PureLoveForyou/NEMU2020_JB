#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include "nemu.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* translate vitual address into lnaddr */
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg_id){
	if(cpu.cr0.protect_enable == 0)
		return addr;
	else
		return cpu.sreg[sreg_id].base + addr;//protect mode needs to translate
}

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int L1_1st_line = ReadCache_L1(addr);
	uint32_t offset = addr & (CACHE_BLOCK_SIZE_L1 - 1);
	uint8_t ret[BURST_LEN << 1];
	if (offset + len > CACHE_BLOCK_SIZE_L1){
		int L1_2nd_line = ReadCache_L1(addr + CACHE_BLOCK_SIZE_L1 - offset);
		memcpy(ret,Cache_L1[L1_1st_line].data + offset,CACHE_BLOCK_SIZE_L1 - offset);
		memcpy(ret + CACHE_BLOCK_SIZE_L1 - offset,Cache_L1[L1_2nd_line].data,len - (CACHE_BLOCK_SIZE_L1 - offset));
	}else {
		memcpy(ret,Cache_L1[L1_1st_line].data + offset, len);
	}

	int temp = 0;
	uint32_t ans = unalign_rw(ret + temp, 4) & (~0u >> ((4 - len) << 3));
	return ans;
	//return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	WriteCache_L1(addr, len, data);
	//dram_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}

