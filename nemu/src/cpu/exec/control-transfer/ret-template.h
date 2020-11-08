#include "cpu/exec/template-start.h"

#define instr ret


make_helper(concat(ret_o_, SUFFIX)) {
    cpu.eip = MEM_R(reg_l(R_ESP));//Get return address
    reg_l(R_ESP) += DATA_BYTE;//update esp
    print_asm_template1();
    return 1;//lenth of instruction ret
}

make_helper(concat(ret_i_, SUFFIX)) {
    int i, val = instr_fetch(cpu.eip + 1, 2);
    cpu.eip = MEM_R(REG(R_ESP));//Get return address
    if(DATA_BYTE == 2)
        cpu.eip &= 0xffff;
    REG(R_ESP) += DATA_BYTE;//update esp
    for(i = 0; i < val; i += DATA_BYTE)
        MEM_W(REG(R_ESP) + i, 0);//pop imm16 bytes of parameters
    REG(R_ESP) += val;//update esp
    print_asm_template1();
    return 1;//lenth of instruction ret
}

#include "cpu/exec/template-end.h"