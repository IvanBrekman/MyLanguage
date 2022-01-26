//
// Created by IvanBrekman on 19.01.2022
//

#ifndef STANDARD_FUNCTIONSH
#define STANDARD_FUNCTIONSH

#include "../Backend/AssemblerGenerator/asm_gen.h"

int print(FILE* asm_file);
int enter(FILE* asm_file);
int sqrt (FILE* asm_file);
int prec (FILE* asm_file);

const StandardFunction ALL_STANDARD_FUNCTIONS[] = {
    { "print",     1, print, 1 },
    { "enter",     2, enter, 0, 0 },
    { "sqrt",      3, sqrt,  1, 1 },
    { "precision", 4, prec,  1, 1 }
};
const int STD_FUNC_AMOUNT = sizeof(ALL_STANDARD_FUNCTIONS) / sizeof(ALL_STANDARD_FUNCTIONS[0]);

#endif // STANDARD_FUNCTIONSH
