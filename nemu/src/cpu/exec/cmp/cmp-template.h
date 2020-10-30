#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
    DATA_TYPE result = op_dest->val - op_src->val;

    cpu.CF = op_src->val > op_dest->val;
    cpu.SF = MSB(result);
    cpu.ZF = !result;
    int Sign_of_dest = MSB(op_dest->val);
    int Sign_of_src = MSB(op_src->val);
    cpu.OF = ( Sign_of_dest != Sign_of_src) && (cpu.SF == Sign_of_src);
    
    /*judge whether number of 1 in low 8 bits is even*/
    result ^= result >>4;
    result ^= result >>2;
    result ^= result >>1;
    cpu.PF = !(result & 1);
    print_asm_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)

#endif
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)
make_instr_helper(i2a)

#include "cpu/exec/template-end.h"