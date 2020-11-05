#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute() {
    OPERAND_W(op_src, MEM_R(reg_l(R_ESP)));//get data at top of stack
    //MEM_W(reg_l(R_ESP), 0);
    reg_l(R_ESP) += DATA_BYTE;//update esp
    print_asm_template1();
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"