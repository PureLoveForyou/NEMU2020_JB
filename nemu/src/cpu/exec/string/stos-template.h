#include "cpu/exec/template-start.h"

#define instr stos

make_helper(concat(stos_n_, SUFFIX)) {
    reg_l(R_EDI) = REG(R_EAX);
    /*update edi*/
    if(cpu.DF == 0)
        reg_l(R_EDI) += DATA_BYTE;
    else
        reg_l(R_EDI) -= DATA_BYTE;
    print_asm("stos%s", str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"