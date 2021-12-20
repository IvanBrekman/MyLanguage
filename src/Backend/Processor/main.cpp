//
// Created by ivanbrekman on 04.10.2021.
//

#include <cstdio>
#include <cstdlib>

#include "config.h"
#include "run_cpu.h"

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

    int exit_code = run_cpu(source_file, executable_file);

    if (argc < 3) {
        FREE_PTR(source_file,     char);
        FREE_PTR(executable_file, char);
    }
    
    return exit_code;
}
