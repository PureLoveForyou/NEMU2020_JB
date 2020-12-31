#include "cpu/exec/template-start.h"

#define instr dec

static void do_execute () {
	DATA_TYPE result = op_src->val - 1;
	OPERAND_W(op_src, result);

	/* TODO: Update EFLAGS. */
	//panic("please implement me");
	cpu.CF = op_src->val < 1;
	cpu.SF = MSB(result);
	cpu.ZF = !result;
	int Sign_of_dest = 0;
    int Sign_of_src = MSB(op_src->val);
    cpu.OF = ( Sign_of_dest != Sign_of_src) && (cpu.SF == Sign_of_dest);
    /*judge whether number of 1 in low 8 bits is even*/
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result & 1);
	print_asm_template1();
}

make_instr_helper(rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
#endif

#include "cpu/exec/template-end.h"
