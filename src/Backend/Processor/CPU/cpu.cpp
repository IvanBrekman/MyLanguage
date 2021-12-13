//
// Created by ivanbrekman on 15.10.2021.
//

/* !NOTE! if you changed arg types (which bit show type or add/delete new types) you must change bin view of jump-type command. Search comment in this file */

#include <cerrno>
#include <cassert>
#include <cstring>
#include <unistd.h>

#include "../../../../config.h"
#include "../../../../libs/baselib.h"
#include "../../../../libs/file_funcs.h"

#include "../arch/helper.h"
#include "../arch/commands.h"
#include "../arch/labels.h"

#include "cpu.h"

Labels labels = { };

int main(int argc, char** argv) {
    if (argc < 2) {
        printf(RED "Cant parse executable file path\n" NATURAL);
        return INVALID_SYNTAX;
    }

    LOG1(printf("------start executing------\n"););

    int exit_code = execute(argv[1]);
    printf(BLUE "Program finished with exit code %d (%s)\n" NATURAL, exit_code, exit_code_desc(exit_code));
    
    LOG1(printf("------end   executing------\n\n"););

    return exit_code;
}

int execute(const char* execute_file) {
    Processor* processor = init_processor();

    int n_commands = -1;
    errno = 0;
    BinCommand* mcodes = read_mcodes(execute_file, &n_commands);
    if (errno != 0) {
        printf(RED "%s\n" NATURAL, error_desc(errno));
        return exit_codes::INVALID_SYNTAX;
    }

    int exit_code = execute_commands(mcodes, n_commands, processor);

    destroy_processor();
    return exit_code;
}

int execute_commands(BinCommand* mcodes, int n_commands, Processor* processor) {
    LOG1(printf("Executing commands:\n"););
    LOG1(labels_ctor(&labels););

    LOG1(printf("Fill labels..\n");                                     // run by all commands to fill labels names
        int ip = processor->ip;
        for ( ; processor->ip < n_commands; processor->ip++) {
            BinCommand b_command = mcodes[processor->ip];

            if (ALL_COMMANDS[b_command.sgn.cmd].args_type == 0b0000000000000010 /* BIN VIEW OF JUMP-TYPE */ && get_lab_by_val(&labels, b_command.argv[LABEL_BIT]) == -1) {  // write label if command is jump-type command
                char name[MAX_ARG_SIZE] = "label_";
                strcat(name, to_string(labels.labels_count));

                write_label(&labels, strdup(name), b_command.argv[LABEL_BIT]);
            }
        }
        processor->ip = ip;
    );

    for ( ; processor->ip < n_commands; processor->ip++) {
        LOG2(printf("ip: %d\n", processor->ip););
        
        BinCommand b_command = mcodes[processor->ip];
        CommandParameters command = ALL_COMMANDS[b_command.sgn.cmd];
        int com_size = sizeof(ALL_COMMANDS) / sizeof(ALL_COMMANDS[0]);

        if (b_command.sgn.cmd < 0 || b_command.sgn.cmd >= com_size) {
            printf(RED "Incorrect command code (%d)\n" NATURAL, b_command.sgn.cmd);
            return exit_codes::INVALID_SYNTAX;
        }

        LOG1(if (LOG_PROCESSOR_STATE == 1) {
                printf("\n");
                processor_dump();
            }
            print_command(&b_command, processor->ip, stdout, &labels);
        );
        int exit_code = command.execute_func((int)b_command.args_type, b_command.argv);

        if (exit_code != exit_codes::OK) {
            switch (exit_code)
            {
            case exit_codes::BREAK:
                printf(RED "Program aborted with exit code %d\n" NATURAL, exit_code);
                assert(0 && "Intentional abort");break;
            case exit_codes::EXIT:
                return exit_codes::OK;
            default:
                break;
            }
        }
        sleep(SLEEP);
    }

    LOG1(labels_dtor(&labels););
    LOG1(printf("\nProcessor final state:\n");
         processor_dump();
    );

    return exit_codes::NO_OBVIOUS_END;
}
