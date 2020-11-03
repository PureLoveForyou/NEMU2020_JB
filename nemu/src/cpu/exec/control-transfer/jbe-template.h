#include "cpu/exec/template-start.h"

#define instr jbe

static void do_execute() {
    DATA_TYPE_S displacement = op_src->val;//get offset
    print_asm("jbe %x", cpu.eip + displacement + DATA_BYTE + 1);
    if(cpu.CF == 1 || cpu.ZF == 1)
        cpu.eip += displacement;//update eip, jump
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"