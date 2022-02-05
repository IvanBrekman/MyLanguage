//
// Created by ivanbrekman on 25.10.2021.
//

/*
Definition of jumps with conditions:
    jump name
    condition sign

    COND_JUMP_DEFINITION(my_name, ==)
*/

COND_JUMP_DEFINITION(jg,  >)
COND_JUMP_DEFINITION(jl,  <)
COND_JUMP_DEFINITION(je,  ==)
COND_JUMP_DEFINITION(jne, !=)
COND_JUMP_DEFINITION(jge, >=)
COND_JUMP_DEFINITION(jle, <=)
