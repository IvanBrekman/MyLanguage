//
// Created by IvanBrekman on 19.01.2022
//

#include "config.h"

#include "functions.h"

#define ADD_FUNC_DESC(format...) {         \
    SPR_FPUTS(asm_file, format);           \
    SPR_FPUTS(asm_file, "\n");             \
}

#define ADD_ASM_CODE(format...) {          \
    SPR_FPUTS(asm_file, "    ");           \
    SPR_FPUTS(asm_file, format);           \
    SPR_FPUTS(asm_file, "\n");             \
}

int print(FILE* asm_file) {
    ASSERT_IF(VALID_PTR(asm_file), "Invalid asm_file ptr", 0);

    ADD_FUNC_DESC("# standard 'print' function");
    ADD_FUNC_DESC("print:");

    ADD_ASM_CODE("pop ax");
    ADD_ASM_CODE("push ax");
    ADD_ASM_CODE("push ax\n");

    ADD_ASM_CODE("outv");
    ADD_ASM_CODE("ret");

    ADD_FUNC_DESC("# end of 'print' function\n");

    return 1;
}

int enter(FILE* asm_file) {
    ASSERT_IF(VALID_PTR(asm_file), "Invalid asm_file ptr", 0);

    ADD_FUNC_DESC("# standard 'enter' function");
    ADD_FUNC_DESC("enter:");

    ADD_ASM_CODE("inv");
    ADD_ASM_CODE("ret");

    ADD_FUNC_DESC("# end of 'enter' function\n");

    return 1;
}

int sqrt(FILE* asm_file) {
    ASSERT_IF(VALID_PTR(asm_file), "Invalid asm_file ptr", 0);

    ADD_FUNC_DESC("# standard 'sqrt' function");
    ADD_FUNC_DESC("sqrt:");

    ADD_ASM_CODE("sqrt");
    ADD_ASM_CODE("ret");

    ADD_FUNC_DESC("# end of 'sqrt' function\n");

    return 1;
}

int prec(FILE* asm_file) {
    ASSERT_IF(VALID_PTR(asm_file), "Invalid asm_file ptr", 0);

    ADD_FUNC_DESC("# standard 'precision' function");
    ADD_FUNC_DESC("precision:");

    ADD_ASM_CODE("pop $prec");
    ADD_ASM_CODE("ret");

    ADD_FUNC_DESC("# end of 'precision' function\n");

    return 1;
}
