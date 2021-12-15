//
// Created by IvanBrekman on 13.12.2021
//

#include <cstring>

#include "../../../config.h"
#include "../../../libs/file_funcs.h"

#include "../front.h"
#include "token.h"

#define DATA data[index]
#define SET_NEW_TOKEN(_type, value) {                           \
    data_store->data[data_store->size] = {                      \
        .type  = _type,                                         \
        /* data */   0,                                         \
                                                                \
        .left  = NULL,                                          \
        .right = NULL                                           \
    };                                                          \
    data_store->data[data_store->size++].data.value = value;    \
}

Tokens* get_tokens(const char* filename) {
    ASSERT_IF(VALID_PTR(filename), "Invalid file ptr", NULL);

    char* data = get_raw_text(filename);
    LOG2(printf("Raw program: '%s'\n", data););

    replace(data, strlen(data), '\n', ' ');
    LOG2(printf("Clear program: '%s'\n\n", data););

    LOG1(printf("Getting tokens...-------------------------\n"););
    Tokens* tokens = take_tokens(data);
    LOG1(printf("Return tokens-----------------------------\n"););

    return tokens;
}

Tokens* take_tokens(const char* data) {
    ASSERT_IF(VALID_PTR(data), "Invalid data ptr", NULL);

    Tokens* data_store = NEW_PTR(Tokens);
    data_store->data   = (Lexem*) calloc_s(MAX_LEXEMS_AMOUNT, sizeof(Lexem));
    data_store->size   = 0;

    int index = 0;
    while (DATA != '\0') {
        int tmp_index = index;

        index += get_number  (&DATA, data_store);
        index += get_name    (&DATA, data_store);
        index += get_operator(&DATA, data_store);

        ASSERT_IF(tmp_index != index, "Index hasn't changed. Some error in take tokens function.", NULL);
    }

    return data_store;
}

int get_number(const char* data, Tokens* data_store) {
    ASSERT_IF(VALID_PTR(data),       "Invalid data ptr",       0);
    ASSERT_IF(VALID_PTR(data_store), "Invalid data_store ptr", 0);

    int index = 0;
    int number = 0;

    while (IS_DIGIT(DATA)) {
        number = number * 10 + (DATA - '0');
        index++;
    }
    if (index > 0) {
        LOG2(printf("Got a number: '%s'\n"
                    "               %d\n", data, number););
        SET_NEW_TOKEN(data_type::CONST_T, number);
    }

    while (DATA == ' ') index++;

    return index;
}

int get_name(const char* data, Tokens* data_store) {
    ASSERT_IF(VALID_PTR(data),       "Invalid data ptr",       0);
    ASSERT_IF(VALID_PTR(data_store), "Invalid data_store ptr", 0);

    int  index = 0;
    char* name = (char*) calloc_s(MAX_NAME_SIZE, sizeof(char));

    while (IS_LATIN(DATA) || DATA == '_') {
        name[index] = DATA;
        index++;
    }
    name[index] = '\0';

    if (index > 0) {
        LOG2(printf("Got a name:   '%s'\n"
                    "              '%s'\n", data, name););
        SET_NEW_TOKEN(data_type::VAR_T, name);
    }

    while (DATA == ' ') index++;

    return index;
}

int get_operator(const char* data, Tokens* data_store) {
    ASSERT_IF(VALID_PTR(data),       "Invalid data ptr",       0);
    ASSERT_IF(VALID_PTR(data_store), "Invalid data_store ptr", 0);

    int  index = 0;
    char* name = (char*) calloc_s(MAX_NAME_SIZE, sizeof(char));

    while (!IS_DIGIT(DATA) && !IS_LATIN(DATA) && DATA != '_' && DATA != ' ') {
        name[index] = DATA;
        index++;
    }
    name[index] = '\0';

    if (index > 0) {
        LOG2(printf("Got an oper:  '%s'\n"
                    "              '%s'\n", data, name););
        SET_NEW_TOKEN(data_type::OPP_T, name);
    }

    while (DATA == ' ') index++;

    return index;
}

int tokens_dump(const Tokens* tokens, const char* reason, FILE* log) {
    ASSERT_IF(VALID_PTR(tokens), "Invalid tokens ptr", 0);
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr",    0);

    fprintf(log, COLORED_OUTPUT("|-------------------------         Tokens  Dump         -------------------------|\n", ORANGE, log));
    FPRINT_DATE(log);
    fprintf(log, COLORED_OUTPUT("%s\n", BLUE, log), reason);

    fprintf(log, "Tokens amount: %d %s\n", tokens->size, tokens->size >= 0 ? "" : BAD_OUTPUT);
    for (int i = 0; i < tokens->size; i++) {
        Lexem cur_lex = tokens->data[i];
        fprintf(log, "Lexem %3d: type - %d %s\n", i, cur_lex.type, cur_lex.type != data_type::ERROR_T ? "" : BAD_OUTPUT);

        fprintf(log, "           data - " PURPLE);
        if      (cur_lex.type == data_type::CONST_T) fprintf(log, "%d\n", cur_lex.data.number);
        else if (cur_lex.type == data_type::VAR_T ||
                 cur_lex.type == data_type::OPP_T)   fprintf(log, "'%s'\n", cur_lex.data.name);
        else                                         fprintf(log, "%s\n", COLORED_OUTPUT("Unknown data type.\n", RED, log));
        fprintf(log, NATURAL "\n");

        fprintf(log, "           left: %p %s\n"
                     "          right: %p %s\n\n",
                cur_lex.left,  cur_lex.left  ? "" : COLORED_OUTPUT("(NULL)", BLUE, log),
                cur_lex.right, cur_lex.right ? "" : COLORED_OUTPUT("(NULL)", BLUE, log));
    }

    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}
