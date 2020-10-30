#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    DATA_TYPE_S displacement = op_src->val;
    print_asm_template1();
    if(cpu.ZF == 1)
        cpu.eip = cpu.eip + displacement;
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"