//
// Created by ivanbrekman on 16.10.2021.
//

#include "config.h"
#include "libs/baselib.h"
#include "libs/stack.h"

#include "helper.h"
#include "commands.h"
#include "src/constants.h"

#define MAX(num1, num2) ((num1 > num2) ? num1 : num2)

Processor processor = { };

Processor* init_processor() {
    processor.ip = 0;

    stack_ctor(processor.stack);
    stack_ctor(processor.call_stack);

    registers_ctor(&processor.regs, REG_NAMES);
    write_to_reg(&processor.regs, system_registers::VALUE_PRECISION, ACCURACY);
    write_to_reg(&processor.regs, system_registers::BASE_POINTER,    CONST_MEMORY_START);

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
    printf(PURPLE "Processor dump---------------------------------------------------------------------\n" NATURAL);

    printf("\nVRAM:       [");
    for (int i = 0; i < 50; i++) {
        printf("%d", processor.RAM[VRAM_START + i]);
        if (i + 1 < 50) printf(", ");
    }
    printf("]\n");

    printf("\nRAM\n");
    RAM_dump(0, CONST_MEMORY_START, 0);
    for (int i = PRINTED_RAM_BLOCKS - 1; i >= 0; i--) RAM_dump(CONST_MEMORY_START, LOCALS_PER_STACK_FRAME, i);

    printf("Registers:\n");
    print_reg(&processor.regs);

    printf("\nStack:      ");
    print_stack_line(&processor.stack, ", ", "\n", log);

    printf("Stack_call: ");
    print_stack_line(&processor.call_stack, ", ", "\n", log);

    printf(PURPLE "-----------------------------------------------------------------------------------\n" NATURAL);

    return exit_codes::OK;
}

int RAM_dump(int start_index, int step, int block_number) {
    const char* color = CYAN;
    int start = start_index + block_number * step;
    int end   = start + step;

    int block_length = 12;
    for (int i = start; i < end; i++) {
        block_length += MAX(digits_number(processor.RAM[i]), digits_number(i));
        if (i + 1 < end) block_length += 2;
    }

    printf("%s", color);
    for (int i = 0; i < block_length / 2 - 6; i++) printf("#");
    printf(" Block №%2d ", block_number);
    for (int i = 0; i < block_length / 2 - 6 + 1; i++) printf("#");
    printf(NATURAL "\n");

    printf("%s#" NATURAL "     ", color);
    for (int i = start; i < end; i++) {
        int shift = MAX(digits_number(processor.RAM[i]), digits_number(i));
        printf("%*d", shift, i);
        if (i + 1 < end) printf("  ");
    } printf("     %s#\n" NATURAL, color);

    printf("%s#" NATURAL "     ", color);
    for (int i = start; i < end; i++) {
        if (processor.RAM[i]) printf(PURPLE);

        int shift = MAX(digits_number(processor.RAM[i]), digits_number(i));
        printf("%*d", shift, processor.RAM[i]);

        if (processor.RAM[i]) printf(NATURAL);

        if (i + 1 < end) printf("  ");
    } printf("     %s#\n" NATURAL, color);

    printf("%s", color);
    for (int i = 0; i < block_length; i++) printf("#");
    printf(NATURAL "\n\n");

    return block_length;
}

int get_processor_ip() {
    return processor.ip;
}
int set_processor_ip(int value) {
    processor.ip += value;
    return processor.ip;
}

// Commands implementation-----------------------------------------------------
#define COMMAND_DEFINITION(name, argc_min, argc_max, argv_m, body) int execute_ ## name(int args_type, int* argv) body
    #include "commands_definition.h"
#undef COMMAND_DEFINITION
// ----------------------------------------------------------------------------
