//
// Created by ivanbrekman on 16.10.2021.
//

#ifndef SIMPLEPROCESSOR_COMMANDS_H
#define SIMPLEPROCESSOR_COMMANDS_H

#include "../../../../libs/stack.h"

#include "helper.h"
#include "registers.h"

struct Processor {
    Stack      stack  = { };
    Stack call_stack  = { };

    int ip            = -1;         // instruction pointer

    Registers regs    = { };        // regs names: ax, bx, cx, dx
    int RAM[RAM_SIZE] = { };
};

struct CommandParameters {
    const char* name = "";
    int code = -1;

    int argc_min  =  0;             // minimum argc amount           
    int argc_max  = -1;             // maximum argc amount
    int args_type =  0;             // number, represented args types (sets with 0b number)

    int (*execute_func) (int args_type, int* argv);
};

// Processor constructor/deconstructor and processor info funtions-------------
Processor* init_processor();
int     destroy_processor();

int get_processor_ip();
int set_processor_ip(int value);

int processor_dump(FILE* log=stdout);
// ----------------------------------------------------------------------------

// Commands prototypes---------------------------------------------------------
#define COMMAND_DEFINITION(name, code, argc_min, argc_max, argv_m, body) int execute_ ## name(int args_type, int* argv);
    #include "commands_definition.h"
#undef COMMAND_DEFINITION
// ----------------------------------------------------------------------------

// Commands definition in array------------------------------------------------
#define COMMAND_DEFINITION(name, code, argc_min, argc_max, argv, body) { #name, code, argc_min, argc_max, argv, execute_ ## name },
const CommandParameters ALL_COMMANDS[] {
    #include "commands_definition.h"
};
#undef COMMAND_DEFINITION
// ----------------------------------------------------------------------------

const int UNKNOWN = -1;                 /* unknown command code */

#endif //SIMPLEPROCESSOR_COMMANDS_H
