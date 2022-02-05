//
// Created by IvanBrekman on 19.01.2022
//

#include <cstring>

#include "config.h"
#include "../Backend/AssemblerGenerator/asm_genDSL.h"

#include "functions.h"

int print(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'print' function");
    ADD_ASM_CODE("print:");

    ASM_INDENT += 4;
    ADD_ASM_CODE("pop  ax");
    ADD_ASM_CODE("push ax");
    ADD_ASM_CODE("pop  bx\n");
    ADD_ASM_CODE("__IB__start_print__:");
    ASM_INDENT += 2;

    ADD_ASM_CODE("push ax");
    ADD_ASM_CODE("push 0");
    ADD_ASM_CODE("je __IB__finish_print__\n");

    ADD_ASM_CODE("out");
    ADD_ASM_CODE("push ax");
    ADD_ASM_CODE("push 1");
    ADD_ASM_CODE("sub");
    ADD_ASM_CODE("pop ax");
    ADD_ASM_CODE("jmp __IB__start_print__\n");

    ASM_INDENT -= 2;
    ADD_ASM_CODE("__IB__finish_print__:");
    ASM_INDENT += 2;

    ADD_ASM_CODE("new");
    ADD_ASM_CODE("push bx");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 6;

    FULL_LINE_COMMENT("End of 'print' function\n");

    return 1;
}

int enter(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'enter' function");
    ADD_ASM_CODE("enter:");

    ASM_INDENT += 4;

    ADD_ASM_CODE("pop");
    PART_LINE_COMMENT("Remove args amount from stack");

    ADD_ASM_CODE("in");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 4;

    FULL_LINE_COMMENT("End of 'enter' function\n");

    return 1;
}

int sqrt(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'sqrt' function");
    ADD_ASM_CODE("sqrt:");

    ASM_INDENT += 4;

    ADD_ASM_CODE("pop");
    PART_LINE_COMMENT("Remove args amount from stack");

    ADD_ASM_CODE("sqrt");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 4;

    FULL_LINE_COMMENT("End of 'sqrt' function\n");

    return 1;
}

int abs(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'abs' function");
    ADD_ASM_CODE("abs:");

    ASM_INDENT += 4;

    ADD_ASM_CODE("pop");
    PART_LINE_COMMENT("Remove args amount from stack");

    ADD_ASM_CODE("abs");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 4;

    FULL_LINE_COMMENT("End of 'abs' function\n");

    return 1;
}

int prec(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'precision' function");
    ADD_ASM_CODE("precision:");

    ASM_INDENT += 4;

    ADD_ASM_CODE("pop");
    PART_LINE_COMMENT("Remove args amount from stack");
    
    ADD_ASM_CODE("push $prec");
    PART_LINE_COMMENT("Check if prec value is already changed");

    ADD_ASM_CODE("push 0");
    ADD_ASM_CODE("jne __IB__break_prec__\n");

    ADD_ASM_CODE("pop  $prec");

    ADD_ASM_CODE("push $prec");
    PART_LINE_COMMENT("Check if prec value is negative");

    ADD_ASM_CODE("push 0");
    ADD_ASM_CODE("jl __IB__break_prec__");
    ADD_ASM_CODE("jmp __IB__next_precision__\n");

    ADD_ASM_CODE("__IB__break_prec__:");
    ADD_ASM_CODE("    abrt\n");

    ADD_ASM_CODE("__IB__next_precision__:");

    FULL_LINE_COMMENT("Update bp value");
    ADD_ASM_CODE("push 1");
    ADD_ASM_CODE("push $bp");
    ADD_ASM_CODE("dmul");
    ADD_ASM_CODE("pop  $bp");
    FULL_LINE_COMMENT("~~~~~~~~~~~~~~~\n");

    ADD_ASM_CODE("push $prec");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 4;

    FULL_LINE_COMMENT("End of 'precision' function\n");

    return 1;
}
