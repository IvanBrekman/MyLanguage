//
// Created by ivanbrekman on 15.10.2021.
//

#ifndef SIMPLEPROCESSOR_ASM_H
#define SIMPLEPROCESSOR_ASM_H

#include "libs/file_funcs.h"
#include "../arch/helper.h"

//! Function execute assembling of source_file in executable_file
//! \param source_file     ptr to source file
//! \param execurable_file ptr to executable file
//! \param label_assembly  is it assembly for detectecting labels (0 or 1)
//! \return                exit code of assembling program
//! \note if label assembly switch to 1 program will check commands and fill label, but won`t write bin commands to file
int assembly(const char* source_file, const char* executable_file, int label_assembly);

//! Function split commands (Text) to arguments (Text of Texts)
//! \param data ptr to object of Text with commands
//! \return     ptr of Text of Texts with splitted commands
Text*                   get_tcom(Text* data);

//! Function checks all commands for correctness
//! \param tcom           ptr to splitted commands
//! \param n_commands     amount of commands
//! \param label_assembly is it assembly for detectecting labels (0 or 1)
//! \return               incorrect command if error was detected, else tcom[0], also program write error to errno
//! \note tcom[0] can`t signalized that all is good (because tcom[0] can also be incorrect command), but if all is good, errno value wouldn`t be changed.
//!       If label_assembly switch to 0, function wouldn`t react at repeated label definition (cause it checked at label_assembly = 1 and label_values wouldn`t celared after it)
Text                  check_tcom(const Text* tcom, int n_commands, int label_assembly);

//! Function convert text commands to binary
//! \param commands   ptr to Text object of commands
//! \param n_commands ptr to value, where commands amount will be written
//! \return           ptr to BinCommand object of commands
BinCommand* get_mcodes_from_tcom(const Text* commands, int* n_commands);

//! Function parse string argument to detect argument types
//! \param arg  ptr to parsed arg
//! \param argv ptr to array, where values of each arg types will be written (default NULL)
//! \return     number, which represents arg types (each arg type write in special bit of this number)
int parse_arg(const char* arg, int* argv=NULL);

#endif //SIMPLEPROCESSOR_ASM_H
