#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    DATA_TYPE_S displacement = op_src->val;//get offset
    print_asm("je %x", cpu.eip + displacement + DATA_BYTE + 1);
    if(cpu.ZF == 1)
        cpu.eip = cpu.eip + displacement;//jump
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"