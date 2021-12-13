//
// Created by ivanbrekman on 15.10.2021.
//

#ifndef SIMPLEPROCESSOR_HELPER_H
#define SIMPLEPROCESSOR_HELPER_H

#include <cmath>

#include "../processor_config.h"

// Registers-------------------------------------------------------------------
static const char* REG_NAMES[] = { "ax", "bx", "cx", "dx", "$prec" };
enum system_registers {
    VALUE_PRECISION = 4
};
// ----------------------------------------------------------------------------

// Processor constants---------------------------------------------------------
const int MAX_REGISTERS = sizeof(REG_NAMES) / sizeof(REG_NAMES[0]);
const int MAX_LABELS    = 50;
const int RAM_SIZE      = 4096;
const int VRAM_START    = 2048;

const int TYPES_AMOUNT = 4;
const int LABEL_BIT    = 0;
const int NUMBER_BIT   = 1;
const int REGISTER_BIT = 2;
const int RAM_BIT      = 3;

const int BITS_TO_ARGV = 2;
const int MAX_ARGV     = TYPES_AMOUNT * BITS_TO_ARGV;
// ----------------------------------------------------------------------------

// Exit codes and errors-------------------------------------------------------
enum exit_codes {
    OK             =  0,
    EXIT           = -1,
    BREAK          = -2,
    INVALID_SYNTAX = -3,
    NO_OBVIOUS_END = -4
};
enum compile_errors {
    UNKNOWN_COMMAND         = -15,
    INCORRECT_ARG_AMOUNT    = -16,
    INCORRECT_ARG_TYPE      = -17,
    REPEAT_LABEL_DEFINITION = -18,
    UNKNOWN_LABEL           = -19
};
enum binary_errors {
    DAMAGED_BINARY          = -63,
    READ_PARTIAL_HEADER     = -64,
    INCORRECT_VERSION       = -65
};
// ----------------------------------------------------------------------------

// BinCommand structure--------------------------------------------------------
struct signature_ {
    unsigned argc : BITS_TO_ARGV;
    unsigned cmd  : 8 - BITS_TO_ARGV;
};

struct BinCommand {
    signature_ sgn     = {};
    int args_type      =  0;
    int argv[MAX_ARGV] = {};
};
// ----------------------------------------------------------------------------

// Description functions-------------------------------------------------------
const char* error_desc(int error_code);
const char* exit_code_desc(int exit_code);
// ----------------------------------------------------------------------------

//! Function print command content
//! \param cmd     ptr to BinCommand object of command
//! \param cmd_num order number of command
//! \param log     ptr to FILE, where command log will be written (default stdout)
//! \param lab     ptr to Labels object (casts to void to avoid looping with including) to print advanced command log with labels names (default NULL)
//! \return        NULL
void print_command (BinCommand* cmd,  int cmd_num, FILE* log=stdout, void* lab=NULL);

int         command_type(const char* command);          /* string command -> command code   */
const char* command_desc(int command);                  /* command code   -> string command */

//! Function make string view of bin argument
//! \param mcode     ptr to BinCommand object of command
//! \param arg_shift index of real argument in BinCommand (means what index of command if you imagine it like string command)
//! \param lab       ptr to Labels object (casts to void to avoid looping with including) to print advanced command log with labels names (default NULL)
//! \return          ptr to string - description of argument
char* arg_desc(const BinCommand* mcode, int arg_shift, void* lab=NULL);

// Read/write bin command functions--------------------------------------------
BinCommand* read_mcodes(const char* executable_file, int* n_commands);
int         write_mcodes(const BinCommand* mcodes, int n_commands, const char* executable_file);
// ----------------------------------------------------------------------------

#endif //SIMPLEPROCESSOR_HELPER_H
