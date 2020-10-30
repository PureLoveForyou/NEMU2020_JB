#include "cpu/exec/template-start.h"

#define instr ret


make_helper(concat(ret_o_, SUFFIX)) {
    cpu.eip = MEM_R(reg_l(R_ESP));//Get return address
    reg_l(R_ESP) += DATA_BYTE;//update esp
    print_asm_template1();
    return 1;//lenth of instruction ret
}

#include "cpu/exec/template-end.h"