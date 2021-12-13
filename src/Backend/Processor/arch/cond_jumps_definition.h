//
// Created by ivanbrekman on 25.10.2021.
//

/*
Definition of jumps with conditions:
    jump name
    command code
    condition sign

    COND_JUMP_DEFINITION(my_name, 32, ==)
*/

COND_JUMP_DEFINITION(jg,  20, >)
COND_JUMP_DEFINITION(jl,  21, <)
COND_JUMP_DEFINITION(je,  22, ==)
COND_JUMP_DEFINITION(jne, 23, !=)
COND_JUMP_DEFINITION(jge, 24, >=)
COND_JUMP_DEFINITION(jle, 25, <=)
