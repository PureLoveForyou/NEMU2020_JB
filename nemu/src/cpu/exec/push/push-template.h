#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
    if(DATA_BYTE == 2) {
        /*operandsize is 2 byte*/
        cpu.esp -= 2;
        MEM_W(cpu.esp, op_src->val);
    }
	else{
        /*operandsize is 4 byte*/
        cpu.esp -= 4;
        if(DATA_BYTE == 1)
            op_src->val = (int8_t)op_src->val;
        MEM_W(reg_l(R_ESP), op_src->val);
    }
    print_asm_template1();
}

make_instr_helper(r);

#include "cpu/exec/template-end.h"
