#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
    DATA_TYPE displacement = op_src->val;//get offset
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

#include "cpu/exec/template-end.h"