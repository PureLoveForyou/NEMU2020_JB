#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_n_, SUFFIX)) {
    uint32_t dest, src = MEM_R(reg_l(R_EDI));
    DATA_TYPE result;
    if(DATA_BYTE == 1)
        dest = reg_b(R_AL);
    else if(DATA_BYTE == 2)
        dest = reg_w(R_AX);
    else
        dest = reg_l(R_EAX);
    result = dest - src;
    /*update edi*/
    if(cpu.DF == 0)
        REG(R_EDI) += DATA_BYTE;
    else
        REG(R_EDI) -= DATA_BYTE;
    /*update CF ZF OF SF PF*/
    cpu.CF = src > dest;
    cpu.SF = MSB(result);
    cpu.ZF = !result;
    int Sign_of_dest = MSB(op_dest->val);
    int Sign_of_src = MSB(op_src->val);
    cpu.OF = ( Sign_of_dest != Sign_of_src) && (cpu.SF == Sign_of_src);
    /*judge whether number of 1 in low 8 bits is even*/
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result & 1);
    print_asm("scas");
    return 1;
}

#include "cpu/exec/template-end.h"