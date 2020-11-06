#include "cpu/exec/template-start.h"

#define instr lods

make_helper(concat(lods_n_, SUFFIX)) {
    if(DATA_BYTE == 1)
        reg_b(R_AL) = MEM_R(REG(R_ESI));
    else if(DATA_BYTE == 2)
        reg_w(R_AX) = MEM_R(REG(R_ESI));
    else
        reg_l(R_EAX) = MEM_R(REG(R_ESI));
    /*update esi*/
    if(cpu.DF == 0)
        REG(R_ESI) += DATA_BYTE;
    else
        REG(R_ESI) -= DATA_BYTE;
    print_asm("lods");
    return 1;
}

#include "cpu/exec/template-end.h"