//
// Created by ivanbrekman on 15.10.2021.
//

#include <cassert>
#include <cstring>
#include <cctype>
#include <cerrno>

#include "config.h"
#include "libs/baselib.h"
#include "libs/file_funcs.h"

#include "../arch/helper.h"
#include "../arch/commands.h"
#include "../arch/labels.h"

#include "asm.h"

Labels labels = {};

int main(int argc, char** argv) {
    if (argc < 3) {
        printf(RED "Cant parse source file path or executable file path\n" NATURAL);
        return INVALID_SYNTAX;
    }

    LOG1(printf("------start assembly------\n"););
    labels_ctor(&labels);

    LOG1(printf("First assembly\n"););
    int exit_code = assembly(argv[1], argv[2], 1);

    LOG1(printf("\nLabels:\n");
         print_labels(&labels);
         printf("\n");
    );

    if (exit_code != exit_codes::OK) {
        return exit_code;
    }

    LOG1(printf("Second assembly\n"););
    exit_code = assembly(argv[1], argv[2], 0);

    LOG1(printf("------end   assembly------\n\n"););

    labels_dtor(&labels);
    return exit_code;
}

int assembly(const char* source_file, const char* executable_file, int label_assembly) {
    assert(VALID_PTR(source_file)     && "Incorrect source_file ptr");
    assert(VALID_PTR(executable_file) && "Incorrect executable_file ptr");

    if (!label_assembly) {                                                  // Checks that all labels has each place to jump
        for (int i = 0; i < labels.labels_count; i++) {
            if (!can_read_label(&labels, i)) {
                errno = compile_errors::UNKNOWN_LABEL;
                printf(RED "%s\n" NATURAL, error_desc(errno));
                return exit_codes::INVALID_SYNTAX;
            }
        }
    }

    Text text = get_text_from_file(source_file, SKIP_EMPTY_STRINGS, SKIP_FISRT_LAST_SPACES);
    
    Text* text_commands = get_tcom(&text);
    int      n_commands = (int)text.lines;

    LOG1(printf("All commands (asm):\n");
        for (int i = 0; i < n_commands; i++) {
            print_text(&text_commands[i], ", ");
            text_commands[i].lines;
        }
        printf("\n");
    );

    errno = 0;
    Text wrong_command = check_tcom(text_commands, n_commands, label_assembly);
    LOG1(printf("Check_tcom result: %d\n", errno););
    if (errno != 0) {
        int err = errno;
        printf(RED "Incorrect command '");
        print_text(&wrong_command, ", ", "");
        printf("'\n%s\n" NATURAL, error_desc(err));

        return exit_codes::INVALID_SYNTAX;
    }

    LOG1(printf("Assembled commands:\n"););
    BinCommand* mcodes = get_mcodes_from_tcom(text_commands, &n_commands);

    if (!label_assembly) {
        write_mcodes(mcodes, n_commands, executable_file);
    }
    
    FREE_PTR(text_commands, Text);
    FREE_PTR(mcodes, BinCommand);
    return exit_codes::OK;
}

Text* get_tcom(Text* data) {
    assert(VALID_PTR(data) && "Invalid data ptr");

    Text* commands = (Text*)calloc(data->lines, sizeof(Text));

    int comment_str = 0;
    for (int i = 0; i < data->lines; i++) {
        char* str_command = data->text[i].ptr;

        if (str_command[0] == COMMENT_SYMBOL) {                             // If string starts with comment symbol skip this string
            comment_str++;
            continue;
        }
        
        char* com_sym_index = strchr(str_command, COMMENT_SYMBOL);
        if (com_sym_index != NULL) {                                        // If find comment symbol in string (not at start), skip checking least string
            data->text[i].len = (int)(com_sym_index - &str_command[0]);

            int spaces = 0;
            for (int cmd_index = data->text[i].len - 1; isspace(data->text[i].ptr[cmd_index]); cmd_index--) {
                spaces++;
            }
            data->text[i].len -= spaces;
            str_command[data->text[i].len] = '\0';
        }

        int args = replace(str_command, data->text[i].len, ' ', '\0') + 1;

        char* insert_ptr = str_command;
        char*  check_ptr = str_command;
        for (int _ = 0; _ < data->text[i].len; _++) {
            if (!(*check_ptr == '\0' && *(check_ptr - 1) == '\0')) {
                *insert_ptr = *check_ptr;
                insert_ptr++;
            } else args--;
            check_ptr++;
        } *insert_ptr = '\0';

        Text command = {
                str_command,
                data->text[i].len + 1,
                (String*)calloc(args, sizeof(String)),
                (size_t)args
        };
        load_string_pointers(&command);

        commands[i - comment_str] = command;
    }

    data->lines -= comment_str;
    return commands;
}

Text check_tcom(const Text* tcom, int n_commands, int label_assembly) {
    Labels tmp_lab = { };
    labels_ctor(&tmp_lab);

    for (int i = 0; i < n_commands; i++) {
        Text cmd = tcom[i];

        int cmd_code = command_type(cmd.text[0].ptr);

        if (cmd_code == UNKNOWN) {                                          // Checks for possible label
            if (possible_label(&labels, cmd.text[0].ptr) == -1) {
                errno = compile_errors::UNKNOWN_COMMAND;
                return cmd;
            }
            if (possible_label(&tmp_lab, cmd.text[0].ptr) == 0 && label_assembly) {
                errno = compile_errors::REPEAT_LABEL_DEFINITION;
                return cmd;
            }

            char* label = strdup(cmd.text[0].ptr);
            char* ptr = strchr(label, ':');
            *ptr = '\0';
            write_label(&tmp_lab, label, 0);
            continue;
        }

        if ((cmd.lines - 1) < ALL_COMMANDS[cmd_code].argc_min || (cmd.lines - 1) > ALL_COMMANDS[cmd_code].argc_max) {
            errno = compile_errors::INCORRECT_ARG_AMOUNT;
            return cmd;
        }

        for (int arg = 1; arg < cmd.lines; arg++) {
            int res_types = parse_arg(cmd.text[arg].ptr);
            LOG2(printf("parse_arg result: %d\n\n", res_types););
            if (res_types == -1 || !extract_bit(ALL_COMMANDS[cmd_code].args_type, res_types)) {
                errno = compile_errors::INCORRECT_ARG_TYPE; 
                return cmd;
            }
        }
    }

    return tcom[0];
}

BinCommand* get_mcodes_from_tcom(const Text* commands, int* n_commands) {
    BinCommand* bit_cmd = (BinCommand*)calloc(*n_commands, sizeof(BinCommand));

    int cmd_index = 0;
    for (int i = 0; i < *n_commands; i++) {
        Text text_cmd = commands[i];
        BinCommand cmd = {};

        int command_code = command_type(text_cmd.text[0].ptr);
        if (command_code == UNKNOWN) {                                      // Means that it is label
            String label = text_cmd.text[0];
            label.len--;
            label.ptr[label.len] = '\0';

            write_label(&labels, text_cmd.text[0].ptr, cmd_index);
            LOG1(print_labels(&labels););
            continue;
        }
        assert(command_code != UNKNOWN && "Unknown command in get_mcodes_from_tcom!! Check check_tcom func!");

        cmd.sgn = { (unsigned)(text_cmd.lines - 1), (unsigned)command_code };
        for (int arg = 1; arg < text_cmd.lines; arg++) {
            int arg_type  = parse_arg(text_cmd.text[arg].ptr, cmd.argv + (TYPES_AMOUNT * (arg - 1)));
            cmd.args_type = arg_type;
        }
        LOG2(print_labels(&labels););

        bit_cmd[cmd_index++] = cmd;

        LOG1(print_command(&cmd, cmd_index - 1););
    }

    *n_commands -= labels.labels_count;                                     // All labels wouldn`t get in bin commands, so we should change n_commands
    return bit_cmd;
}

int parse_arg(const char* arg, int* argv) {
    char      name[MAX_ARG_SIZE] = { };
    char const_val[MAX_ARG_SIZE] = { };
    LOG2(printf("arg: %s\n", arg););

    Registers reg_tmp = {};
    registers_ctor(&reg_tmp, REG_NAMES);
    
    int cond    = (arg[0] == '[');
    int arg_len = strlen(arg) - 2 * cond;
    int is_ram  = cond << RAM_BIT;
    arg = arg + cond;                                                       // If arg starts with '[', it is RAM address, with shifting arg ptr to generalize parsing 

    int parse_len = 0;
    int argc = sscanf(arg, "%[a-zA-Z_$]+%[0-9]%n", name, const_val, &parse_len);
    LOG2(printf( "[a-zA-Z_$]+[0-9]\n"
                "sscanf res : %d\n"
                "name       : \"%s\"\n"
                "const_value: \"%s\"\n"
                "parse len  : %d\n"
                "arg len    : %d\n\n",
                argc, name, const_val, parse_len, arg_len);
    );
    if (argc == 2 && get_reg_by_name(&reg_tmp, name) != -1 && parse_len == arg_len) {
        if (VALID_PTR(argv)) {
            argv[REGISTER_BIT] = get_reg_by_name(&reg_tmp, name);
            argv[NUMBER_BIT]   = atoi(const_val);
        }
        return is_ram + (1 << REGISTER_BIT) + (1 << NUMBER_BIT) + 0;
    }

    argc = parse_len = 0;
    argc = sscanf(arg, "%[a-zA-Z_$0-9]%n", name, &parse_len);
    LOG2(printf( "[a-zA-Z_$0-9]\n"
                "sscanf res : %d\n"
                "name       : \"%s\"\n"
                "const_value: \"%s\"\n"
                "parse len  : %d\n"
                "arg len    : %d\n\n",
                argc, name, const_val, parse_len, arg_len);
    );
    if (argc == 1 && (parse_len == arg_len) && !IS_DIGIT(arg[0])) {
        if (get_reg_by_name(&reg_tmp, name) != -1) {
            if (VALID_PTR(argv)) {
                argv[REGISTER_BIT] = get_reg_by_name(&reg_tmp, name);
            }
            return is_ram + (1 << REGISTER_BIT) + 0 + 0;
        } else {
            if (VALID_PTR(argv)) {
                int value = read_label(&labels, name);
                argv[LABEL_BIT] = value;
                write_label(&labels, strdup(name), value);
                LOG1(print_labels(&labels););
            }
            return is_ram + 0 + 0 + 1;
        }
    }

    argc = parse_len = 0;
    argc = sscanf(arg, "%[-?0-9]%n", const_val, &parse_len);
    LOG2(printf( "[0-9]\n"
                "sscanf res : %d\n"
                "name       : \"%s\"\n"
                "const_value: \"%s\"\n"
                "parse len  : %d\n"
                "arg len    : %d\n",
                argc, name, const_val, parse_len, arg_len);
    );
    if (argc == 1 && (strlen(const_val) == arg_len)) {
        if (VALID_PTR(argv)) {
            argv[NUMBER_BIT] = atoi(const_val);
        }
        return is_ram + (0 << REGISTER_BIT) + (1 << NUMBER_BIT) + 0;
    }

    return -1;
}
