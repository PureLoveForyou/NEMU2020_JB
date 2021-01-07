#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
    DATA_TYPE_S displacement = op_src->val;//get offset
    if(op_src->type == OP_TYPE_IMM) {
        cpu.eip += (DATA_TYPE_S)displacement;//update eip, jump
        print_asm("jmp %x", cpu.eip + 1 + DATA_BYTE);
    }
    else {
        cpu.eip = displacement - concat(decode_rm_, SUFFIX)(cpu.eip + 1) - 1;
        print_asm_template1();
    }
}

make_instr_helper(i)
make_instr_helper(rm)

#if DATA_BYTE == 4

extern Sreg_Descriptor sreg_info;
Sreg_Descriptor tmp;

make_helper(ljmp){
    sreg_info = tmp;
    cpu.eip = instr_fetch(cpu.eip+1, 4) - 7;
    cpu.cs.selector = instr_fetch(cpu.eip+1 + 4, 2);

    uint16_t idx = cpu.cs.selector >> 3;//index of sreg
	lnaddr_t chart_addr = cpu.gdtr.base + (idx << 3);//chart addr
	sreg_info.part1 = lnaddr_read(chart_addr, 4);
	sreg_info.part2 = lnaddr_read(chart_addr + 4, 4);

	cpu.cs.base = sreg_info.base1 + (sreg_info.base2 << 16) + (sreg_info.base3 << 24);
	cpu.cs.limit = sreg_info.limit1 + (sreg_info.limit2 << 16) + (0xfff << 24);
	if (sreg_info.g == 1) {	//g=0,1b; g=1,4kb, 2^12
		cpu.cs.limit <<= 12;
	}
	// printf("get here!\n");
    print_asm("ljmp 0x%x 0x%x",instr_fetch(cpu.eip+1 + 4, 2),instr_fetch(cpu.eip+1, 4));
    return 7;    
}
#endif

#include "cpu/exec/template-end.h"