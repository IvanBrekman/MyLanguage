//
// Created by IvanBrekman on 13.12.2021
//

#include "config.h"

#include "libs/baselib.h"
#include "src/Frontend/front.h"

int main(int argc, char** argv) {
    char* source_filepath = (char*) calloc_s(MAX_FILEPATH_SIZE, sizeof(char));

    if (argc > 1) {
        source_filepath = argv[1];
    } else {
        printf("Input source filepath\n");
        scanf("%s", source_filepath);
    }

    Front(source_filepath);

    if (argc <= 1) {
        FREE_PTR(source_filepath, char);
    }

    return 0;
}
