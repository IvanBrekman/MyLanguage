//
// Created by ivanbrekman on 17.10.2021.
//

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include "run_cpu.h"

// Array of programs, which time of last change will be checked
const char* TRACKED_PROGRAMS[] = {
        "arch/commands.cpp",  "arch/commands.h",
        "arch/helper.cpp",    "arch/helper.h",
        "arch/labels.cpp",    "arch/labels.h",
        "arch/registers.cpp", "arch/registers.h",
        "arch/commands_definition.h",

        "asm/asm.cpp", "asm/asm.h",
        "dis/dis.cpp", "dis/dis.h",
        "CPU/cpu.cpp", "CPU/cpu.h",

        "../../../libs/baselib.cpp",    "../../../libs/baselib.h",
        "../../../libs/file_funcs.cpp", "../../../libs/file_funcs.h",
        "../../../libs/stack.cpp",      "../../../libs/stack.h",

        "../../../config.h"
};

const char* decompile_output = "commands_disassembled.txt";
const char* compile_strings[] = {
        "gcc asm/asm.cpp ../../../libs/baselib.cpp ../../../libs/file_funcs.cpp ../../../libs/stack.cpp arch/helper.cpp arch/commands.cpp arch/labels.cpp arch/registers.cpp -lm -o asm/asm.cat",
        "gcc dis/dis.cpp ../../../libs/baselib.cpp ../../../libs/file_funcs.cpp ../../../libs/stack.cpp arch/helper.cpp arch/commands.cpp arch/labels.cpp arch/registers.cpp -lm -o dis/dis.cat",
        "gcc CPU/cpu.cpp ../../../libs/baselib.cpp ../../../libs/file_funcs.cpp ../../../libs/stack.cpp arch/helper.cpp arch/commands.cpp arch/labels.cpp arch/registers.cpp -lm -o CPU/cpu.cat"
};
const char* execute_strings[] = {
        "./asm/asm.cat ",
        "./dis/dis.cat ",
        "./CPU/cpu.cat "
};

//! Function compile assembler, disassembler and CPU programs (if it need), then compile and decompile source_file (if it need), and finelly execute executable_file
//! \param source_file     path to file with asm commands
//! \param executable_file path to executable file, which will be created (if it need) and exeuted
//! \return                exit code of executing program
//! \note Compiling programs depends on time of last change in files
int run_cpu(const char* source_file, const char* executable_file) {
    assert(VALID_PTR(source_file)     && "Invalid ptr to source_file.");
    assert(VALID_PTR(executable_file) && "Invalid ptr to source_file.");

    char* command_compile   = (char*) calloc(strlen(execute_strings[0]) + strlen(source_file) + 1 + strlen(executable_file) + 1, sizeof(char));
    strcpy(command_compile, execute_strings[0]);
    strcat(command_compile, source_file);
    strcat(command_compile, " ");
    strcat(command_compile, executable_file);
    printf("command compile:   %s\n", command_compile);

    char* command_decompile = (char*) calloc(strlen(execute_strings[1]) + strlen(executable_file) + 1 + strlen(decompile_output) + 1, sizeof(char));
    strcpy(command_decompile, execute_strings[1]);
    strcat(command_decompile, executable_file);
    strcat(command_decompile, " ");
    strcat(command_decompile, decompile_output);
    printf("command decompile: %s\n", command_decompile);

    char* command_execute   = (char*) calloc(strlen(execute_strings[2]) + strlen(source_file) + 1, sizeof(char));
    strcpy(command_execute, execute_strings[2]);
    strcat(command_execute, executable_file);
    printf("command execute:   %s\n\n", command_execute);

    CHECK_PROCESSOR_SYSTEM_CALL(system(compile_strings[0]), "asm/asm.cat");
    CHECK_PROCESSOR_SYSTEM_CALL(system(compile_strings[1]), "dis/dis.cat");
    CHECK_PROCESSOR_SYSTEM_CALL(system(compile_strings[2]), "CPU/cpu.cat");

    CHECK_SYSTEM_CALL(system(command_compile),   source_file, executable_file);
    CHECK_SYSTEM_CALL(system(command_decompile), source_file, decompile_output);
    printf("\n");

    int exit_code = system(command_execute);

    FREE_PTR(command_compile,   char);
    FREE_PTR(command_decompile, char);
    FREE_PTR(command_execute,   char);
    return exit_code;
}
