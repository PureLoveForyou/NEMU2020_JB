#include "nemu.h"
#include <stdlib.h>
#include <time.h>

CPU_state cpu;

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
	srand(time(0));
	uint32_t sample[8];
	uint32_t eip_sample = rand();
	cpu.eip = eip_sample;

	int i;
	for(i = R_EAX; i <= R_EDI; i ++) {
		sample[i] = rand();
		reg_l(i) = sample[i];
		assert(reg_w(i) == (sample[i] & 0xffff));
	}

	assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
	assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
	assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
	assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
	assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
	assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
	assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
	assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

	assert(sample[R_EAX] == cpu.eax);
	assert(sample[R_ECX] == cpu.ecx);
	assert(sample[R_EDX] == cpu.edx);
	assert(sample[R_EBX] == cpu.ebx);
	assert(sample[R_ESP] == cpu.esp);
	assert(sample[R_EBP] == cpu.ebp);
	assert(sample[R_ESI] == cpu.esi);
	assert(sample[R_EDI] == cpu.edi);

	assert(eip_sample == cpu.eip);
}

void sreg_set(uint8_t sreg_id){
	Assert(cpu.cr0.protect_enable, "Not in protect mode!");
	uint16_t idx = cpu.sreg[sreg_id].selector >> 3;//The index of sreg
	lnaddr_t chart_addr = cpu.gdtr.base + (idx << 3);//chart addr

	sreg_info.part1 = lnaddr_read(chart_addr, 4);
	sreg_info.part2 = lnaddr_read(chart_addr + 4, 4);

	Assert(sreg_info.p == 1, "Segement Not Exist!");

	cpu.sreg[sreg_id].base = sreg_info.base1 + (sreg_info.base2 << 16) + (sreg_info.base3 << 24);
	cpu.sreg[sreg_id].limit = sreg_info.limit1 + (sreg_info.limit2 << 16) + (0xfff << 24);
	if(sreg_info.g == 1){
		/* g == 0, 1B; g== 1, 4 KB(2^12) */
		cpu.sreg[sreg_id].limit <<= 12;
	}
}