#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
    
}

make_instr_helper(r2rm);

#include "cpu/exec/template-end.h"
