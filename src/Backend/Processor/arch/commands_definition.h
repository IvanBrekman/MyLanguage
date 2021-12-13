//
// Created by ivanbrekman on 17.10.2021.
//

#include "DSL/commands_syntax.h"

/*
This file includes all definitions of processor commands

Each command has:
    name
    number code
    number of minimum args amount
    number of maximum args amount
    number, which shows allowed types of arguments **
    executing code in {}

    ** this number has the form:
        0bnnnnnnnnnnnnnnnn, where n = 0 or 1

       m-th bit of this number means if the argument can have m-th combination in tryth table of types
       We can build tryth table from all types of args (let it will be 3 types)
         ABC
       0 000
       1 001
       2 010
       3 011
       4 100
       5 101
       6 110
       7 111

       For exmaple if 3-th bit of number is 1, it means that arguments can have type B and type C arguments

       0b1111110011111100 (allowed RAM, register, number types and their combinations)
    
    COMMAND_DEFINITION( name, 37, 0, 2, 0b0101010101010101, {
        printf("This is test command\n");
        return OK_;
    })

You also can use DSL to create your commands or write them directly accessing to processor and other commands
*/

// Break commands--------------------------------------------------------------
COMMAND_DEFINITION( hlt,   0, 0, 0, 0b0000000000000000,  {
    return EXIT_;
})

COMMAND_DEFINITION( abrt,  1, 0, 0, 0b0000000000000000,  {
    return BREAK_;
})
// ----------------------------------------------------------------------------

// Stack commands--------------------------------------------------------------
COMMAND_DEFINITION( push,  2, 1, 1, 0b1111110011111100,  {
    int arg = 0;
    if (HAS_REGISTER) arg += read_from_reg(REG, ARG(0, REGISTER_BIT));
    if (HAS_NUMBER)   arg += ARG(0, NUMBER_BIT);
    if (HAS_RAM)      arg  = RAM(arg);
    
    PUSH(arg);

    return OK_;
})

COMMAND_DEFINITION( pop,   3, 0, 1, 0b1111110000110000,  {
    int pop_value = POP;

    if (args_type > 0) {
        if (HAS_RAM) {
            int arg = 0;
            if (HAS_REGISTER) arg += read_from_reg(REG, ARG(0, REGISTER_BIT));
            if (HAS_NUMBER)   arg += ARG(0, NUMBER_BIT);
            RAM(arg) = pop_value;
        } else {
            write_to_reg(REG, ARG(0, REGISTER_BIT), pop_value);
        }
    }

    return OK_;
})

COMMAND_DEFINITION( vrf,   4, 0, 0, 0b0000000000000000,  {
    int err = Stack_error(STACK);
    printf("Stack Verify: %s (%d)\n", Stack_error_desc(err), err);

    return OK_;
})

COMMAND_DEFINITION( dump,  5, 0, 0, 0b0000000000000000,  {
    DUMP;
    return OK_;
})

COMMAND_DEFINITION( prt,   6, 0, 0, 0b0000000000000000,  {
    PRINT;
    return OK_;
})
// ----------------------------------------------------------------------------

// Arithmetic commands--------------------------------------------------------
COMMAND_DEFINITION( add,   7, 0, 0, 0b0000000000000000,  {
    PUSH(POP + POP);
    return OK_;
})

COMMAND_DEFINITION( sub,   8, 0, 0, 0b0000000000000000,  {
    int arg1 = POP;
    int arg2 = POP;

    PUSH(arg2 - arg1);
    return OK_;
})

COMMAND_DEFINITION( mul,   9, 0, 0, 0b0000000000000000,  {
    PUSH(POP * POP);
    return OK_;
})

COMMAND_DEFINITION( div,  10, 0, 0, 0b0000000000000000,  {
    int arg1 = POP;
    int arg2 = POP;

    if (arg1 == 0) {
        printf(RED "Zero division error\n" NATURAL);
        return BREAK_;
    }

    PUSH(arg2 / (arg1 * pow(10, -PRECISION_)));
    return OK_;
})

COMMAND_DEFINITION( mod,  11, 1, 1, 0b0000000000000100,  {
    PUSH(POP % ARG(0, NUMBER_BIT));
    return OK_;
})

COMMAND_DEFINITION( sqr,  12, 0, 0, 0b0000000000000000,  {
    int arg1 = POP;

    PUSH(arg1 * arg1);
    return OK_;
})

COMMAND_DEFINITION( sqrt, 13, 0, 0, 0b0000000000000000,  {
    int arg1 = POP;

    if (arg1 < 0) {
        printf(RED "Sqrt from negative number error\n" NATURAL);
        return BREAK_;
    }

    PUSH((int)sqrt(arg1));
    return OK_;
})

COMMAND_DEFINITION( abs,  14, 0, 0, 0b0000000000000000,  {
    int arg1 = POP;

    PUSH(arg1 >= 0 ? arg1 : -arg1);
    return OK_;
})
// ----------------------------------------------------------------------------

// IO commands-----------------------------------------------------------------
COMMAND_DEFINITION( in,   15, 0, 0, 0b0000000000000000,  {
    printf("Input number..\n");
    
    int num = poisons::UNINITIALIZED_INT;
    while (scanf("%d", &num) != 1) {
        printf("NaN\n");
        while (getchar() != '\n') continue;
    }

    PUSH(num);
    return OK_;
})

COMMAND_DEFINITION( inv,  16, 0, 0, 0b0000000000000000,  {
    printf("Input number..\n");
    
    int num = poisons::UNINITIALIZED_INT;
    while (scanf("%d", &num) != 1) {
        printf("NaN\n");
        while (getchar() != '\n') continue;
    }

    PUSH(num * (int)pow(10, PRECISION_));
    return OK_;
})

COMMAND_DEFINITION( out,  17, 0, 0, 0b0000000000000000,  {
    OUT;
    return OK_;
})

COMMAND_DEFINITION( outv, 18, 0, 0, 0b0000000000000000,  {
    OUTV;
    return OK_;
})
// ----------------------------------------------------------------------------

// Jump-types commands---------------------------------------------------------
COMMAND_DEFINITION( jmp,  19, 1, 1, 0b0000000000000010,  {
    IP = ARG(0, LABEL_BIT) - 1;
    return IP;
})

#define COND_JUMP_DEFINITION(name, code, sign)                                  \
COMMAND_DEFINITION( name, code, 1, 1, 0b0000000000000010, {                     \
    int first  = POP;                                                           \
    int second = POP;                                                           \
                                                                                \
    if (second sign first) {                                                    \
        IP = ARG(0, LABEL_BIT) - 1;                                             \
    }                                                                           \
                                                                                \
    return IP;                                                                  \
})
#include "cond_jumps_definition.h"
#undef COND_JUMP_DEFINITION
/* !NOTE! last command code in cond_jumps_definition.h is       25      !NOTE! */

COMMAND_DEFINITION( call, 26, 1, 1, 0b0000000000000010,  {
    PUSH_C(IP + 1);

    IP = ARG(0, LABEL_BIT) - 1;
    return IP;
})

COMMAND_DEFINITION( ret,  27, 0, 0, 0b0000000000000000,  {
    IP = POP_C - 1;
    return IP;
})
// ----------------------------------------------------------------------------

// Draw commands---------------------------------------------------------------
COMMAND_DEFINITION( draw, 28, 2, 2, 0b1111110011111100,  {
    int arg1 = 0;
    int arg2 = 0;

    if (HAS_REGISTER) arg1 += read_from_reg(REG, ARG(0, REGISTER_BIT));
    if (HAS_NUMBER)   arg1 += ARG(0, NUMBER_BIT);
    if (HAS_RAM)      arg1  = RAM(arg1);

    if (HAS_REGISTER) arg2 += read_from_reg(REG, ARG(1, REGISTER_BIT));
    if (HAS_NUMBER)   arg2 += ARG(1, NUMBER_BIT);
    if (HAS_RAM)      arg2  = RAM(arg2);

    LOG1(printf("arg1 - arg2: %d - %d\n", arg1, arg2););
    for (int y = 0; y < arg1; y++) {
        for (int x = 0; x < arg2; x++) {
            printf("%s", RAM(VRAM_START + y * arg2 + x) == 0 ? "**" : "  ");
        }
        printf("\n");
    }

    return OK_;
})

COMMAND_DEFINITION( cat,  29, 0, 0, 0b0000000000000000,  {
   printf("____________________$$____________$$_____\n"
          "_____________ _____$___$________$___$____\n"
          "__________________$_____$$$$$$_____ $____\n"
          "__________________$____sss___sss____$____\n"
          "________________ _$____@_____@_____$_____\n"
          "_________________$ _______$$$_______$____\n"
          "_____$$$$$$$$_____$_______^_______$______\n"
          "_ __$$________$______$$_________$$_______\n"
          "____$_________$_____$___$ $$$$$___$______\n"
          "_______$______$____$__$________$__$______\n"
          "_______$____ _$____$__$__________$__$____\n"
          "______$____$___$$$$__$__________$_ _$$$$_\n"
          "_____$___$____$____$__$________$___$___$_\n"
          "_____$__$__ ___$____$__$________$__$____$\n"
          "____$___$______$____$__$____$ _$__$____$_\n"
          "______$__$______$____$___$_$_____$___$___\n"
          "_______$ ___$$$$$_$___$___$_$____$___$___\n"
          "__________$$$$$_$____$____$__ ___$____$__\n"
          "________________$$$_$_____$______$_$$$___\n"
          "_________ ____________$$$$___$$$$$ ______\n");

   return OK_;
})
// ----------------------------------------------------------------------------
