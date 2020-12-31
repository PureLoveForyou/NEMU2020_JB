#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {
    int length = concat(decode_i_, SUFFIX)(eip+1);//length of (instruction-1)
    reg_l(R_ESP) -= DATA_BYTE;//esp - 4
    swaddr_write(reg_l(R_ESP), 4, cpu.eip + length);
    DATA_TYPE_S displacement= op_src->val;//displacement
    print_asm("call %x", cpu.eip + 1 + length + displacement);
    cpu.eip += displacement;//jump
    return length + 1;//return length of instruction
}

make_helper(concat(call_rm_, SUFFIX))
{
    int length = concat(decode_rm_, SUFFIX)(eip+1);//length of (instruction-1)
    reg_l(R_ESP) -= DATA_BYTE;//esp - 4
    swaddr_write(reg_l(R_ESP), 4, cpu.eip + length);
    DATA_TYPE_S displacement= op_src->val;//displacement
    print_asm_template1();
    cpu.eip = displacement - length - 1;//jump
    return length + 1;//return length of instruction
}

#include "cpu/exec/template-end.h"
