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

COND_JUMP_DEFINITION(jg,  18, >)
COND_JUMP_DEFINITION(jl,  19, <)
COND_JUMP_DEFINITION(je,  20, ==)
COND_JUMP_DEFINITION(jne, 21, !=)
COND_JUMP_DEFINITION(jge, 22, >=)
COND_JUMP_DEFINITION(jle, 23, <=)
