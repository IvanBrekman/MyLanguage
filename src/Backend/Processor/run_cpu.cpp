//
// Created by ivanbrekman on 17.10.2021.
//

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include "config.h"
#include "processor_config.h"
#include "run_cpu.h"

#define SPR_SYSTEM_CODE(result, format...) {                                        \
    char* command = (char*) calloc_s(MAX_SPRINTF_STRING_SIZE, sizeof(char));        \
    sprintf(command, format);                                                       \
                                                                                    \
    result = system(command);                                                       \
                                                                                    \
    FREE_PTR(command, char);                                                        \
}


// Array of programs, which time of last change will be checked
const char* TRACKED_PROGRAMS[] = {
        "arch/commands.cpp",  "arch/commands.h",
        "arch/helper.cpp",    "arch/helper.h",
        "arch/labels.cpp",    "arch/labels.h",
        "arch/registers.cpp", "arch/registers.h",
        "arch/commands_definition.h", "arch/DSL/commands_syntax.h",

        "asm/asm.cpp", "asm/asm.h",
        "dis/dis.cpp", "dis/dis.h",
        "CPU/cpu.cpp", "CPU/cpu.h",

        "../../../libs/baselib.cpp",    "../../../libs/baselib.h",
        "../../../libs/file_funcs.cpp", "../../../libs/file_funcs.h",
        "../../../libs/stack.cpp",      "../../../libs/stack.h",

        "../../../config.h", "processor_config.h"
};

const char* decompile_output = "logs/processor_work/commands_disassembled.txt";

//! Function compile assembler, disassembler and CPU programs (if it need), then compile and decompile source_file (if it need), and finelly execute executable_file
//! \param context     ptr to FilesContext object with needed strings
//! \return            exit code of executing program
//! \note Compiling programs depends on time of last change in files
int run_cpu(FilesContext* context) {
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", -1);
    ASSERT_IF(VALID_PTR(context->source_file),     "Invalid source_file ptr",     -1);
    ASSERT_IF(VALID_PTR(context->executable_file), "Invalid executable_file ptr", -1);

    ASSERT_IF(VALID_PTR(context->compile_asm), "Invalid compile_asm ptr", -1);
    ASSERT_IF(VALID_PTR(context->compile_dis), "Invalid compile_dis ptr", -1);
    ASSERT_IF(VALID_PTR(context->compile_cpu), "Invalid compile_cpu ptr", -1);

    char* asm_file = NEW_PTR(char, MAX_SPRINTF_STRING_SIZE);
    sprintf(asm_file, "%s/%s", context->home_dir, "asm/asm.cat");

    char* dis_file = NEW_PTR(char, MAX_SPRINTF_STRING_SIZE);
    sprintf(dis_file, "%s/%s", context->home_dir, "dis/dis.cat");

    char* cpu_file = NEW_PTR(char, MAX_SPRINTF_STRING_SIZE);
    sprintf(cpu_file, "%s/%s", context->home_dir, "CPU/cpu.cat");

    LOG1(printf("asm file: '%s'\ndis file: '%s'\ncpu file: '%s'\n", asm_file, dis_file, cpu_file););

    check_tracked_programs(context, asm_file);
    check_tracked_programs(context, dis_file);
    check_tracked_programs(context, cpu_file);

    if (!context->updated) {
        LOG1(printf(ORANGE "Skip processor compiling" NATURAL "\n"););
    }

    if (file_last_change(context->source_file) > file_last_change(context->executable_file) || context->updated) {
        int exit_code = 0;
        SPR_SYSTEM_CODE(exit_code, "./%s %s %s", asm_file, context->source_file, context->executable_file);
        WAIT_INPUT;

        if (exit_code != 0) return exit_code;
    }
    if (file_last_change(context->source_file) > file_last_change(decompile_output) || context->updated) {
        int exit_code = 0;
        SPR_SYSTEM_CODE(exit_code, "./%s %s %s", dis_file, context->executable_file, decompile_output);
        WAIT_INPUT;

        if (exit_code != 0) return exit_code;
    }
    LOG1(printf("\n"););

    char* command = NEW_PTR(char, MAX_SPRINTF_STRING_SIZE);
    sprintf(command, "./%s %s", cpu_file, context->executable_file);
    int exit_code = system(command);

    FREE_PTR(command,  char);
    FREE_PTR(asm_file, char);
    FREE_PTR(dis_file, char);
    FREE_PTR(cpu_file, char);

    return exit_code;
}

//! Function checked tracked programs and compile processor programs (if it need)
//! \param context         ptr to FilesContext object with needed strings
//! \param regarding_file  ptr to filename regarding which checked tracking programs
//! \return                1 if success else 0
int check_tracked_programs(FilesContext* context, const char* regarding_file) {
    ASSERT_IF(VALID_PTR(context),        "Invalid context ptr",        -1);
    ASSERT_IF(VALID_PTR(regarding_file), "Invalid regarding_file ptr", -1);

    ASSERT_IF(VALID_PTR(context->source_file),     "Invalid source_file ptr",     -1);
    ASSERT_IF(VALID_PTR(context->executable_file), "Invalid executable_file ptr", -1);

    ASSERT_IF(VALID_PTR(context->compile_asm), "Invalid compile_asm ptr", -1);
    ASSERT_IF(VALID_PTR(context->compile_dis), "Invalid compile_dis ptr", -1);
    ASSERT_IF(VALID_PTR(context->compile_cpu), "Invalid compile_cpu ptr", -1);

    if (context->updated) return 1;

    for (const char* tracked_program : TRACKED_PROGRAMS) {
        char* full_file = NEW_PTR(char, MAX_SPRINTF_STRING_SIZE);
        sprintf(full_file, "%s/%s", context->home_dir, tracked_program);

        if (file_last_change(full_file) > file_last_change(regarding_file)) {
            LOG1(printf("Start compiling processor...\n"););

            SYS_COMPILE(context->compile_asm);
            SYS_COMPILE(context->compile_dis);
            SYS_COMPILE(context->compile_cpu);

            LOG1(printf(GREEN "Successful compiling\n\n" NATURAL "\n"););

            context->updated = 1;
            return 1;
        }                                                                                                       
    }

    return 0;
}
