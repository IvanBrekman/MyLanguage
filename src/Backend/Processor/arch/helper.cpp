//
// Created by ivanbrekman on 15.10.2021.
//

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cassert>

#include "../../../../config.h"
#include "../processor_config.h"
#include "../../../../libs/baselib.h"
#include "../../../../libs/file_funcs.h"

#include "helper.h"
#include "commands.h"
#include "labels.h"

const char*     error_desc(int error_code) {
    switch (error_code) {
        case compile_errors::UNKNOWN_COMMAND:
            return "Unknown command";
        case compile_errors::INCORRECT_ARG_AMOUNT:
            return "Invalid arguments amount for this command";
        case compile_errors::INCORRECT_ARG_TYPE:
            return "Incorrect argument type for this command";
        case compile_errors::REPEAT_LABEL_DEFINITION:
            return "Label with this name is already defined";
        case compile_errors::UNKNOWN_LABEL:
            return "No label defined with this name";
        
        case binary_errors::DAMAGED_BINARY:
            return "Data in executable file are damaged";
        case binary_errors::READ_PARTIAL_HEADER:
            return "Cant read full header in executable file";
        case binary_errors::INCORRECT_VERSION:
            return "Version in executable file doesnt match with processor version";
        default:
            return "Unknown error";
    }
}
const char* exit_code_desc(int exit_code)  {
    switch (exit_code)
    {
        case exit_codes::OK:
            return "ok";
        case exit_codes::EXIT:
            return "Default program break";
        case exit_codes::BREAK:
            return "Program was aborted";
        case exit_codes::INVALID_SYNTAX:
            return "Incorrect syntax detected";
        case exit_codes::NO_OBVIOUS_END:
            return "Program wasn`t finished with 'hlt' command. Undefined behavior";
        default:
            return "Unknown exit code";
    }
}


void print_command (BinCommand* cmd, int cmd_num, FILE* log, void* lab) {
    assert(VALID_PTR(cmd) && "Invalid cmd ptr");
    assert(VALID_PTR(log) && "Invalid log ptr");
    assert(cmd_num >= 0   && "Incorrect cmd_num value");

    fprintf(log, "%04d    ", cmd_num);
    fprintf(log, "[ { %02d | %03d } , %04d , ", cmd->sgn.argc, cmd->sgn.cmd, atoi(bin4(cmd->args_type)) /* int view of binary number to fill '0' before number */);
    fprintf(log, "{ ");
    for (int i = 0; i < MAX_ARGV; i++) {
        fprintf(log, "%2d", cmd->argv[i]);
        if (i + 1 < MAX_ARGV && (i + 1) % TYPES_AMOUNT == 0) fprintf(log, " | ");
        else if (i + 1 < MAX_ARGV) fprintf(log, ", ");
    }

    fprintf(log, " } ]    %s ", command_desc(cmd->sgn.cmd));
    for (int i = 0; i < cmd->sgn.argc; i++) {
        fprintf(log, "%s ", arg_desc(cmd, i, lab));
    }

    fprintf(log, "\n");
}

int         command_type(const char* command) {
    assert(VALID_PTR(command) && "Invalid BinCommand ptr");

    for (CommandParameters cmd_par : ALL_COMMANDS) {
        if (strcmp(command, cmd_par.name) == 0) {
            return cmd_par.code;
        }
    }

    return UNKNOWN;
}
const char* command_desc(int command) {
    return ALL_COMMANDS[command].name;
}

char* arg_desc(const BinCommand* mcode, int arg_shift, void* lab) {
    static int labels_count = 0;

    char* arg_string = (char*) calloc(MAX_ARG_SIZE, sizeof(char));
    if (extract_bit(mcode->args_type, RAM_BIT))      strcat(arg_string, "[");
    if (extract_bit(mcode->args_type, REGISTER_BIT)) strcat(arg_string, REG_NAMES[mcode->argv[(arg_shift * TYPES_AMOUNT) + REGISTER_BIT]]);

    if (extract_bit(mcode->args_type, REGISTER_BIT) && extract_bit(mcode->args_type, NUMBER_BIT)) strcat(arg_string, "+");
    
    if (extract_bit(mcode->args_type, NUMBER_BIT))   strcat(arg_string, to_string(mcode->argv[(arg_shift * TYPES_AMOUNT) + NUMBER_BIT]));
    if (extract_bit(mcode->args_type, RAM_BIT))      strcat(arg_string, "]");

    if (extract_bit(mcode->args_type, LABEL_BIT)) {
        strcat(arg_string, "label_");

        int lab_index = VALID_PTR(lab) ? get_lab_by_val((Labels*) lab, mcode->argv[(arg_shift * TYPES_AMOUNT) + LABEL_BIT]) : -1;
        if (lab_index != -1) {
            strcat(arg_string, to_string(lab_index));
        } else {
            strcat(arg_string, to_string(labels_count++));
        }
    }

    return arg_string;
}

BinCommand* read_mcodes(const char* executable_file, int* n_commands) {
    assert(VALID_PTR(executable_file) && "Incorrect executable_file ptr");
    assert(VALID_PTR(n_commands) && "Incorrect n_commands ptr");

    FILE* exe_file = fopen(executable_file, "rb");
    assert(VALID_PTR(exe_file) && "Cant open file with mode wb");

    header received_header = { "", "" };
    int hdr_data = (int)fread(&received_header, sizeof(header), 1, exe_file);

    LOG1(printf( "real signature: %s\n"
                "real version:   %s\n\n"
                "rec signature:  %s\n"
                "rec version:    %s\n\n",
                HEADER.signature, HEADER.version, received_header.signature, received_header.version);
    );
    if (hdr_data != 1) {
        errno = binary_errors::READ_PARTIAL_HEADER;
        return {};
    } else if (strcmp(received_header.signature, HEADER.signature) != 0) {
        errno = binary_errors::DAMAGED_BINARY;
        return {};
    } else if (strcmp(received_header.version, HEADER.version) != 0) {
        errno = binary_errors::INCORRECT_VERSION;
        return {};
    }

    int fsize = file_size(executable_file) - hdr_data * sizeof(header);
    assert(fsize % sizeof(BinCommand) == 0 && "Executable file is damaged");

    *n_commands = fsize / (int)sizeof(BinCommand);
    BinCommand* commands = (BinCommand*) calloc(*n_commands, sizeof(BinCommand));

    int wr_com = (int)fread(commands, sizeof(BinCommand), *n_commands, exe_file);

    LOG1(printf("Read elems: %d\n", wr_com););
    assert(wr_com * sizeof(BinCommand) == fsize && "File size and number of read bytes arent equal");

    fclose(exe_file);
    return commands;
}
int         write_mcodes(const BinCommand* mcodes, int n_commands, const char* executable_file) {
    assert(VALID_PTR(mcodes) && "Incorrect mcodes ptr");
    assert(VALID_PTR(executable_file) && "Incorrect executable_file ptr");

    FILE* exe_file = fopen(executable_file, "wb");
    assert(VALID_PTR(exe_file) && "Cant open file with mode wb");

    LOG1(printf( "signature: %s\n"
                "version:   %s\n\n", HEADER.signature, HEADER.version);
    );
    
    int hd_bts = (int)fwrite(&HEADER, sizeof(HEADER), 1, exe_file);
    fclose(exe_file);

    exe_file = fopen(executable_file, "ab");
    int wr_com = (int)fwrite(mcodes, sizeof(BinCommand), n_commands, exe_file);
    LOG1(printf( "Written elems:    %d\n"
                "n_commands:       %d\n"
                "size BinCommand:  %zd\n\n", wr_com, n_commands, sizeof(BinCommand));
    );

    fclose(exe_file);
    return wr_com;
}
