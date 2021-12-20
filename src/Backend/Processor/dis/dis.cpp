//
// Created by ivanbrekman on 15.10.2021.
//

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cerrno>

#include "config.h"
#include "libs/baselib.h"
#include "libs/file_funcs.h"

#include "../arch/helper.h"
#include "../arch/labels.h"
#include "../arch/commands.h"

#include "dis.h"

Labels labels = { };

int main(int argc, char** argv) {
    if (argc < 3) {
        printf(RED "Cant parse executable file path or source file path" NATURAL "\n");
        return exit_codes::INVALID_SYNTAX;
    }

    LOG1(printf("------start disassembly------\n"););
    int exit_code = disassembly(argv[1], argv[2]);
    LOG1(printf("------end   disassembly------\n\n"););

    return exit_code;
}

int disassembly(const char* executable_file, const char* source_file) {
    assert(VALID_PTR(source_file)     && "Incorrect source_file ptr");
    assert(VALID_PTR(executable_file) && "Incorrect executable_file ptr");

    int n_commands = -1;
    errno = 0;
    BinCommand* mcodes = read_mcodes(executable_file, &n_commands);
    if (errno != 0) {
        printf(RED  "Error while reading mashine codes from executable file\n%s\n" NATURAL, error_desc(errno));
        return exit_codes::INVALID_SYNTAX;
    }

    labels_ctor(&labels);
    Text* tcom = get_tcom_from_mcodes(mcodes, n_commands);

    LOG1(printf("All commands (dis)\n");
        for (int i = 0; i < n_commands; i++) {
            print_text((const Text*)&tcom[i]);
        }
    );

    int label_text = 0;
    for (int i = 0; i < n_commands; i++) {
        for (int lab_index = 0; lab_index < labels.labels_count; lab_index++) {     // Insert label to source_file if need
            if (labels.labels[lab_index] == i) {
                char ** data = (char**) calloc(2, sizeof(char*));
                data[0] = (char*)"";
                data[1] = strdup(strcat((char*)labels.names[lab_index], ":"));
                
                write_strings_to_file(source_file, i == 0 ? "w" : "a", (const char**)data, 2);

                label_text = 1;
                FREE_PTR(data, char*);
                break;
            }
        }

        if (label_text) {                                                           // if this part of code belongs to some label, insert '\t' before command
            char ** data = (char**) calloc(1, sizeof(char*));
            data[0] = (char*)"\t";

            write_strings_to_file(source_file, i == 0 ? "w" : "a", (const char**)data, 1, "", "");

            FREE_PTR(data, char*);
        }
        write_text_to_file(source_file, i == 0 ? "w" : "a", (const Text*)&tcom[i], " ");
    }

    labels_dtor(&labels);
    return exit_codes::OK;
}

Text* get_tcom_from_mcodes(BinCommand* mcodes, int n_commands) {
    assert(VALID_PTR(mcodes) && "Invalid ptr to mcodes");
    assert(n_commands >= 0 && "Negative n_commands value");

    Text* tcom = (Text*) calloc(n_commands, sizeof(Text));

    for (int i = 0; i < n_commands; i++) {
        BinCommand mcode = mcodes[i];
        int n_args = 1 + mcode.sgn.argc;

        char** array = (char**) calloc(n_args, sizeof(char*));
        array[0] = (char*)command_desc((int)mcode.sgn.cmd);
        for (int arg = 0; arg < mcode.sgn.argc; arg++) {
            array[arg + 1] = arg_desc(&mcode, arg, &labels);
        }
        
        Text cmd = convert_to_text((const char**)array, n_args);
        tcom[i] = cmd;

        if (ALL_COMMANDS[mcode.sgn.cmd].args_type == 0b0000000000000010 && get_lab_by_val(&labels, mcode.argv[LABEL_BIT]) == -1) {  // if command is jump-type command, add new label
            char name[MAX_ARG_SIZE] = "label_";
            strcat(name, to_string(labels.labels_count));

            write_label(&labels, strdup(name), mcode.argv[LABEL_BIT]);
        }
    }

    return tcom;
}
