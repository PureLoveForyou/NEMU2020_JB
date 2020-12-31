#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_n_, SUFFIX)) {
    swaddr_t dest = REG(R_EAX), src = MEM_R(reg_l(R_EDI));
    DATA_TYPE result = dest - src;
    /*update edi*/
    if(cpu.DF == 0)
        reg_l(R_EDI) += DATA_BYTE;
    else
        reg_l(R_EDI) -= DATA_BYTE;
    /*update CF ZF OF SF PF*/
    cpu.CF = src > dest;
    cpu.SF = MSB(result);
    cpu.ZF = !result;
    int Sign_of_dest = MSB(dest);
    int Sign_of_src = MSB(src);
    cpu.OF = ( Sign_of_dest != Sign_of_src) && (cpu.SF == Sign_of_src);
    /*judge whether number of 1 in low 8 bits is even*/
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result & 1);
    print_asm("scas%s", str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"