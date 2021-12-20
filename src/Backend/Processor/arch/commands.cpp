//
// Created by ivanbrekman on 16.10.2021.
//

#include "config.h"
#include "libs/baselib.h"
#include "libs/stack.h"

#include "helper.h"
#include "commands.h"

Processor processor = { };

Processor* init_processor() {
    processor.ip = 0;

    stack_ctor(processor.stack);
    stack_ctor(processor.call_stack);

    registers_ctor(&processor.regs, REG_NAMES);
    write_to_reg(&processor.regs, system_registers::VALUE_PRECISION, ACCURACY);

    return &processor;
}
int destroy_processor() {
    processor.ip = -1;

    Stack_dtor_(&processor.stack);
    Stack_dtor_(&processor.call_stack);
    registers_dtor(&processor.regs);

    return 0;
}

int processor_dump(FILE* log) {
    printf("Registers:\n");
    print_reg(&processor.regs);

    printf("Stack:      ");
    print_stack_line(&processor.stack, ", ", "\n", log);

    printf("Stack_call: ");
    print_stack_line(&processor.call_stack, ", ", "\n", log);

    printf("RAM:        [");
    for (int i = 0; i < 50; i++) {
        printf("%d", processor.RAM[i]);
        if (i + 1 < 50) printf(", ");
    }
    printf("]\n");

    printf("VRAM:       [");
    for (int i = 0; i < 50; i++) {
        printf("%d", processor.RAM[VRAM_START + i]);
        if (i + 1 < 50) printf(", ");
    }
    printf("]\n");

    return exit_codes::OK;
}

int get_processor_ip() {
    return processor.ip;
}
int set_processor_ip(int value) {
    processor.ip += value;
    return processor.ip;
}

// Commands implementation-----------------------------------------------------
#define COMMAND_DEFINITION(name, code, argc_min, argc_max, argv_m, body) int execute_ ## name(int args_type, int* argv) body
    #include "commands_definition.h"
#undef COMMAND_DEFINITION
// ----------------------------------------------------------------------------
