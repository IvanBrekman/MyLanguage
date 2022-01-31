//
// Created by ivanbrekman on 20.10.2021.
//

#include <cstdio>
#include <cassert>
#include <cstring>

#include "libs/baselib.h"
#include "registers.h"


int    registers_ctor(Registers* reg, const char* names[]) {
    assert(VALID_PTR(reg)   && "Invalid reg ptr");
    assert(VALID_PTR(names) && "Invalid names ptr");

    for (int i = 0; i < MAX_REGISTERS; i++) {
        reg->regs[i]  = poisons::UNINITIALIZED_INT;
        reg->names[i] = names[i];
    }

    return MAX_REGISTERS;
}
int    registers_dtor(Registers* reg) {
    assert(VALID_PTR(reg) && "Invalid reg ptr");
    
    for (int i = 0; i < MAX_REGISTERS; i++) {
        reg->regs[i]  = poisons::FREED_ELEMENT;
        reg->names[i] = (char*)poisons::FREED_PTR;
    }

    return MAX_REGISTERS;
}
int   get_reg_by_name(Registers* reg, const char* name) {
    assert(VALID_PTR(reg)  && "Invalid reg ptr");
    assert(VALID_PTR(name) && "Invalid name ptr");
    
    for (int i = 0; i < MAX_REGISTERS; i++) {
        if (strcmp(name, reg->names[i]) == 0) {
            return i;
        }
    }

    return -1;
}
int         print_reg(Registers* reg) {
    assert(VALID_PTR(reg) && "Invalid reg ptr");
    
    for (int i = 0; i < MAX_REGISTERS; i++) {
        if (reg->regs[i] != poisons::UNINITIALIZED_INT) {
            printf("%s: % 2d\n", reg->names[i], reg->regs[i]);
        }
    }

    return MAX_REGISTERS;
}


int      write_to_reg(Registers* reg, int reg_index, int value) {
    assert(VALID_PTR(reg) && "Invalid reg ptr");
    assert(0 <= reg_index && reg_index < MAX_REGISTERS && "reg_index out of range");
    
    reg->regs[reg_index] = value;
    return reg->regs[reg_index];
}
int     read_from_reg(Registers* reg, int reg_index) {
    assert(VALID_PTR(reg) && "Invalid reg ptr");

    if (!can_read_from_reg(reg, reg_index)) {
        return poisons::UNINITIALIZED_INT;
    }

    return reg->regs[reg_index];

}
int can_read_from_reg(Registers* reg, int reg_index) {
    assert(VALID_PTR(reg) && "Invalid reg ptr");

    return 0 <= reg_index && reg_index < MAX_REGISTERS && reg->regs[reg_index] != poisons::UNINITIALIZED_INT && reg->regs[reg_index] != poisons::FREED_ELEMENT;
}
