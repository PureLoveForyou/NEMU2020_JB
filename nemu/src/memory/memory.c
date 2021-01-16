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

hwaddr_t page_translate(lnaddr_t addr){
	if(!cpu.cr0.protect_enable || !cpu.cr0.paging) 
		return addr;//No paging mechanism, return directly

	// addr: directory | page | offset
	uint32_t tmp;
	uint32_t dir = addr >> 22;
	uint32_t page = (addr >> 12) & 0x3ff;
	uint32_t offset = addr & 0xfff;

	// directory and second page
	Page_Descriptor first, second;
	
	// get directory
	tmp = (cpu.cr3.page_directory_base << 12) + (dir << 2);
	first.val = hwaddr_read(tmp, 4);
	Assert(first.p == 1, "Directory cannot be used!");
	
	// get page 
	tmp = (first.addr << 12) + (page << 2);
	second.val =  hwaddr_read(tmp, 4);
	Assert(second.p == 1, "Page cannot be used!");
	return (second.addr << 12) + offset;
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
	//return hwaddr_read(addr, len);
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	uint32_t current_offset = addr & 0xfff; //low 12 bit
	if(current_offset + len - 1 > 0xfff){ // cross page boundary
		Assert(0, "Data cross the page boundary!");
		/*size_t l = 0xfff - current_offset + 1;
		uint32_t ar = lnaddr_read(addr,l);
		uint32_t al = lnaddr_read(addr + l,len - l);
		return (al << (l << 3)) | ar;*/
	}else{
		hwaddr_t hwaddr = page_translate(addr);
		return hwaddr_read(hwaddr, len);
	}
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	//hwaddr_write(addr, len, data);
#ifdef DEBUG
 	assert(len == 1 || len == 2 || len == 4);
#endif
	uint32_t current_offset = addr & 0xfff;
	if (current_offset + len - 1 > 0xfff){
		Assert(0,"Data cross the page boundary!");
		/*size_t l = 0xfff - now_offset + 1;
		lnaddr_write(addr,l,data & ((1 << (l << 3)) - 1));
		lnaddr_write(addr + l,len - l,data >> (l << 3));*/
	}else {
		hwaddr_t hwaddr = page_translate(addr);
		hwaddr_write(hwaddr, len, data);
	}
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

