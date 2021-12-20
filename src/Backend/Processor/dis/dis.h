//
// Created by ivanbrekman on 15.10.2021.
//

#ifndef SIMPLEPROCESSOR_DIS_H
#define SIMPLEPROCESSOR_DIS_H

#include "libs/file_funcs.h"
#include "../arch/helper.h"

//! Function execute disassembling of executable_file and write asm commands to source_file
//! \param executable_file path to executable file
//! \param source_file     path to source_file
//! \return                exit code of disassembling
int disassembly(const char* executable_file, const char* source_file);

//! Function convert BinCommands to text asm commands
//! \param mcodes     pointer to object of BinCommand structure
//! \param n_commands amount of bin commands
//! \return           pointer to Text structure of text view of bin commands
Text* get_tcom_from_mcodes(BinCommand* mcodes, int n_commands);

#endif //SIMPLEPROCESSOR_DIS_H
