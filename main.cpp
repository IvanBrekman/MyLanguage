//
// Created by IvanBrekman on 13.12.2021
//

#include "config.h"

#include "libs/baselib.h"
#include "src/Frontend/front.h"
#include "src/Backend/back.h"

int main(int argc, char** argv) {
    char* source_filepath = (char*) calloc_s(MAX_FILEPATH_SIZE, sizeof(char));

    if (argc > 1) {
        source_filepath = argv[1];
    } else {
        printf("Input source filepath\n");
        scanf("%s", source_filepath);
    }

    FrontContext* front_data = Front(source_filepath);
     BackContext*  back_data =  Back(front_data);
    
    LOG1(printf("All ready for work. Run program on processor...\n"););
    WAIT_INPUT;

    if (file_last_change("config.h") > file_last_change("logs/processor_work/processor.out")) {
        system("make pc");
    }

    int work_result = system("./logs/processor_work/processor.out logs/processor_work/asm_code.txt logs/processor_work/out.kitty");
    LOG1(printf(BLUE "Program finished with exit code %d\n" NATURAL, work_result););

    if (argc <= 1) {
        FREE_PTR(source_filepath, char);
    }

    return 0;
}
