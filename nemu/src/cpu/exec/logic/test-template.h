#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
    DATA_TYPE andresult = op_src->val & op_dest->val;

    /*update CF ZF OF SF PF*/
    cpu.CF = 0;
    cpu.ZF = !andresult;
    cpu.OF = 0;
    cpu.SF = MSB(andresult);//get sign flag
    /*judge whether number of 1 in low 8 bits is even*/
    andresult ^= andresult >> 4;
    andresult ^= andresult >> 2;
    andresult ^= andresult >> 1;
    cpu.PF = !(andresult & 1);
    print_asm_template2();
}

make_instr_helper(r2rm);
make_instr_helper(i2rm);
make_instr_helper(i2a);

#include "cpu/exec/template-end.h"
