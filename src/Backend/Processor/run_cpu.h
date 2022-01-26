//
// Created by ivanbrekman on 17.10.2021.
//

#ifndef SIMPLEPROCESSOR_RUN_CPU_H
#define SIMPLEPROCESSOR_RUN_CPU_H

#include "libs/baselib.h"
#include "libs/file_funcs.h"

#define SYS_COMPILE(string) {                           \
    printf("%s\n", string);                             \
    int exit_code = system(string);                     \
    if (exit_code != 0) {                               \
        printf(RED "Error in compiling.\n" NATURAL);    \
        ASSERT_IF(0, "Error in compiling", -1);         \
    }                                                   \
}

struct FilesContext {
    const char*        home_dir = NULL;
    const char*     source_file = NULL;
    const char* executable_file = NULL;

    const char* compile_asm = NULL;
    const char* compile_dis = NULL;
    const char* compile_cpu = NULL;

    int updated = 0;
};

int                run_cpu(FilesContext* context);
int check_tracked_programs(FilesContext* context, const char* regarding_file);

#endif //SIMPLEPROCESSOR_RUN_CPU_H
