#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_n_, SUFFIX)) {
    MEM_W(REG(R_EDI), MEM_R(REG(R_ESI)));
    if(cpu.DF == 0) {
        reg_l(R_EDI) += DATA_BYTE;
        reg_l(R_ESI) += DATA_BYTE;
    }
    else {
        reg_l(R_EDI) -= DATA_BYTE;
        reg_l(R_ESI) -= DATA_BYTE;
    }
    print_asm("movs%s", str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"