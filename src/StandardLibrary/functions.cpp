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
    ADD_ASM_CODE("push ax\n");

    ADD_ASM_CODE("out");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 4;

    FULL_LINE_COMMENT("End of 'print' function\n");

    return 1;
}

int enter(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'enter' function");
    ADD_ASM_CODE("enter:");

    ASM_INDENT += 4;
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
    ADD_ASM_CODE("sqrt");
    ADD_ASM_CODE("ret");
    ASM_INDENT -= 4;

    FULL_LINE_COMMENT("End of 'sqrt' function\n");

    return 1;
}

int prec(ASMGenerateContext* context) {
    ASSERT_CTX;

    FULL_LINE_COMMENT("Standard 'precision' function");
    ADD_ASM_CODE("precision:");

    ASM_INDENT += 4;
    ADD_ASM_CODE("push $prec");
    PART_LINE_COMMENT("Check if prec value is already changed");

    ADD_ASM_CODE("push 0");
    ADD_ASM_CODE("jne break_prec\n");

    ADD_ASM_CODE("pop  $prec");

    ADD_ASM_CODE("push $prec");
    PART_LINE_COMMENT("Check if prec value is negative");

    ADD_ASM_CODE("push 0");
    ADD_ASM_CODE("jl break_prec");
    ADD_ASM_CODE("jmp next_precision\n");

    ADD_ASM_CODE("break_prec:");
    ADD_ASM_CODE("    abrt\n");

    ADD_ASM_CODE("next_precision:");

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
