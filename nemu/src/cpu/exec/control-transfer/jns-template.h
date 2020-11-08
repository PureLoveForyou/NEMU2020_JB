#include "cpu/exec/template-start.h"

#define instr jns

static void do_execute() {
    DATA_TYPE_S displacement = op_src->val;//get offset
    print_asm_template1();
    if(cpu.SF == 0)
        cpu.eip += displacement;//update eip, jump
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"