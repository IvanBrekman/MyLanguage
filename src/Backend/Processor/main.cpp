//
// Created by ivanbrekman on 04.10.2021.
//

#include <cstdio>
#include <cstdlib>

#include "config.h"
#include "run_cpu.h"
#include "src/constants.h"

const char* ASM_COMPILE = "gcc src/Backend/Processor/asm/asm.cpp libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp src/Backend/Processor/arch/helper.cpp src/Backend/Processor/arch/commands.cpp src/Backend/Processor/arch/labels.cpp src/Backend/Processor/arch/registers.cpp -lm -I . -o src/Backend/Processor/asm/asm.cat";
const char* DIS_COMPILE = "gcc src/Backend/Processor/dis/dis.cpp libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp src/Backend/Processor/arch/helper.cpp src/Backend/Processor/arch/commands.cpp src/Backend/Processor/arch/labels.cpp src/Backend/Processor/arch/registers.cpp -lm -I . -o src/Backend/Processor/dis/dis.cat";
const char* CPU_COMPILE = "gcc src/Backend/Processor/CPU/cpu.cpp libs/baselib.cpp libs/file_funcs.cpp libs/stack.cpp src/Backend/Processor/arch/helper.cpp src/Backend/Processor/arch/commands.cpp src/Backend/Processor/arch/labels.cpp src/Backend/Processor/arch/registers.cpp -lm -I . -o src/Backend/Processor/CPU/cpu.cat";

// gcc main.cpp run_cpu.cpp libs/baselib.cpp libs/file_funcs.cpp -o main.out
int main(int argc, char** argv) {
    char* source_file     = (char*) calloc(MAX_FILEPATH_SIZE, sizeof(char));
    char* executable_file = (char*) calloc(MAX_FILEPATH_SIZE, sizeof(char));

    if (argc >= 3) {
        source_file     = argv[1];
        executable_file = argv[2];
    } else {
        printf("Введите путь к исходному файлу\n");
        scanf("%s", source_file);

        printf("Введите путь к исполняемому файлу (в который запишутся команды)\n");
        scanf("%s",executable_file);
    }

    FilesContext* ctx = NEW_PTR(FilesContext, 1);
    *ctx = { HOME_DIR, source_file, executable_file, ASM_COMPILE, DIS_COMPILE, CPU_COMPILE };

    int exit_code = run_cpu(ctx);

    if (argc < 3) {
        FREE_PTR(source_file,     char);
        FREE_PTR(executable_file, char);
    }
    
    printf("%sProcessor finished work with exit code %d\n" NATURAL, (exit_code ? RED : BLUE), exit_code);
    return exit_code;
}
