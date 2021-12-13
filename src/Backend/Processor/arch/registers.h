//
// Created by ivanbrekman on 20.10.2021.
//

#ifndef SIMPLEPROCESSOR_REGISTERS_H
#define SIMPLEPROCESSOR_REGISTERS_H

#include "helper.h"

struct Registers {
    int          regs[MAX_REGISTERS] = { };     // Jump indexes
    const char* names[MAX_REGISTERS] = { };     // Reg names
};

// Register constructor/deconstructor------------------------------------------
int    registers_ctor(Registers* reg,  const char* names[MAX_REGISTERS]);
int    registers_dtor(Registers* reg);
// ----------------------------------------------------------------------------

//! Function get register index by name !NOTE! Works at O(n^2)
//! \param reg  ptr to Registers object
//! \param name ptr to name of register, which we try to find
//! \return     index of suitable register or -1, if there aren`t any suitable registers
//! \note Function use strcmp, so it works very slow
int   get_reg_by_name(Registers* reg,  const char* name);
int         print_reg(Registers* reg);

// Functions for interaction with registers------------------------------------
int      write_to_reg(Registers* reg, int reg_index, int value);
int     read_from_reg(Registers* reg, int reg_index);               /* Function will return poisons::UNINITIALIZED_INT if you can`t read from this register */
// ----------------------------------------------------------------------------

//! Function checks opportunity to read value from register
//! \param reg       ptr to Registers object
//! \param reg_index index of register, which we checks
//! \return          1 if we can read from register, else 0
//! \note function checks reg_index value (0 <= reg_index < MAX_REGISTERS) and value in register with this index (not poisons::UNINITIALIZED_INT and not poisons::FREED_ELEMENT)
int can_read_from_reg(Registers* reg, int reg_index);

#endif //SIMPLEPROCESSOR_REGISTERS_H
