//
// Created by IvanBrekman on 19.01.2022
//

#ifndef STANDARD_FUNCTIONSH
#define STANDARD_FUNCTIONSH

#include "../Backend/AssemblerGenerator/asm_gen.h"

int print(ASMGenerateContext* context);
int enter(ASMGenerateContext* context);
int sqrt (ASMGenerateContext* context);
int prec (ASMGenerateContext* context);

const StandardFunction ALL_STANDARD_FUNCTIONS[] = {
    { "print",     1, print, 1 },
    { "enter",     2, enter, 0, 0 },
    { "sqrt",      3, sqrt,  1, 1 },
    { "precision", 4, prec,  1, 1 }
};
const int STD_FUNC_AMOUNT = sizeof(ALL_STANDARD_FUNCTIONS) / sizeof(ALL_STANDARD_FUNCTIONS[0]);

#endif // STANDARD_FUNCTIONSH
