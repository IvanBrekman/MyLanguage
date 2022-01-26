//
// Created by ivanbrekman on 25.10.2021.
//

/*
DSL for commands
This file added defines for simplyfication accessing to processor and writing new commands

To create new DSL word add new define with name and replaced text:
    #define EXAMPLE printf("example DSL word\n");
*/

#include "config.h"
#include "../../processor_config.h"

#define OK_                     exit_codes::OK
#define EXIT_                   exit_codes::EXIT
#define BREAK_                  exit_codes::BREAK

#define PUSH(val)               push(&processor.stack, val)
#define POP                     pop(&processor.stack)
#define DUMP                    stack_dump(processor.stack, "System dump call")

#define PRINT                   print_stack(&processor.stack)

#define PUSH_C(val)             push(&processor.call_stack, val)
#define POP_C                   pop(&processor.call_stack)
#define IP                      processor.ip

#define RAM(val)                processor.RAM[val]
#define REG                     &processor.regs
#define STACK                   &processor.stack

#define HAS_REGISTER            extract_bit(args_type, REGISTER_BIT)
#define HAS_NUMBER              extract_bit(args_type, NUMBER_BIT)
#define HAS_RAM                 extract_bit(args_type, RAM_BIT)

#define ARG(number, type)       argv[(number * TYPES_AMOUNT) + type]

#define PRECISION_              read_from_reg(REG, system_registers::VALUE_PRECISION)

#define OUT {                                                                                                \
    if (LOG_PRINTF) printf(RED "############################################################\n" NATURAL);    \
    printf("%d\n", POP);                                                                                     \
    if (LOG_PRINTF) printf(RED "############################################################\n" NATURAL);    \
}

#define OUTV {                                                                                               \
    if (LOG_PRINTF) printf(RED "############################################################\n" NATURAL);    \
    printf("%.*f\n", PRECISION_, POP * pow(10, -PRECISION_));                                                \
    if (LOG_PRINTF) printf(RED "############################################################\n" NATURAL);    \
}
