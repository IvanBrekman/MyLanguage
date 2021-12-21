//
// Created by IvanBrekman on 13.12.2021
//

#include <cstring>

#include "config.h"

#include "../front.h"
#include "token.h"

#define DATA         data->text[cur_str].ptr[cur_index]
#define DATA_STRING &data->text[cur_str].ptr[tmp_index]
#define SHIFT       cur_index - tmp_index
#define SET_NEW_TOKEN(_type, value) {                           \
    data_store->data[data_store->size] = {                      \
        .type  = _type,                                         \
        /* data */   0,                                         \
        .string = cur_str + 1                                   \
    };                                                          \
    data_store->data[data_store->size++].data.value = value;    \
}
#define CLEAR_SPACES while (DATA == ' ') cur_index++;

int print_lexem(const Lexem* lex) {
    switch (lex->type) {
        case data_type::CONST_T:
            printf("%d", lex->data.number);
            break;
        case data_type::VAR_T:
        case data_type::OPR_T:
            printf("%s", lex->data.name);
            break;
        
        default:
            printf("\"UN\"");
    }

    return 1;
}

Tokens* get_tokens(const char* filename) {
    ASSERT_IF(VALID_PTR(filename), "Invalid file ptr", NULL);

    Text data = get_text_from_file(filename);
    LOG2(printf("Raw program:\n"); print_text(&data, "\n"););

    LOG1(printf("Getting tokens...-------------------------\n"););
    Tokens* tokens = take_tokens(&data);
    LOG1(printf("Return tokens-----------------------------\n"););

    free_text(&data);
    return tokens;
}

Tokens* take_tokens(const Text* data) {
    ASSERT_IF(VALID_PTR(data), "Invalid data ptr", NULL);

    Tokens* data_store = NEW_PTR(Tokens, 1);
    data_store->data   = (Lexem*) calloc_s(MAX_LEXEMS_AMOUNT, sizeof(Lexem));
    data_store->size   = 0;

    int cur_index = 0, cur_str = 0;
    while (cur_str < (int) data->lines) {
        int tmp_index = cur_index, tmp_string = cur_str;

        cur_index = get_number  (data, data_store, cur_str, cur_index);
        cur_index = get_name    (data, data_store, cur_str, cur_index);
        cur_index = get_operator(data, data_store, cur_str, cur_index);

        if (DATA == '\0') {
            cur_str++;
            cur_index = 0;
        }

        ASSERT_IF(tmp_index != cur_index || tmp_string != cur_str, "cur_index hasn't changed. Some error in take tokens function.", NULL);
    }

    return data_store;
}

int get_number(const Text* data, Tokens* data_store, int cur_str, int cur_index) {
    ASSERT_IF(VALID_PTR(data),       "Invalid data ptr",       0);
    ASSERT_IF(VALID_PTR(data_store), "Invalid data_store ptr", 0);
    
    int number = 0;
    int tmp_index = cur_index;

    while (IS_DIGIT(DATA)) {
        number = number * 10 + (DATA - '0');
        cur_index++;
    }
    if (SHIFT > 0) {
        LOG2(printf("Got a number: '%s'\n"
                    "               %d\n", DATA_STRING, number););
        SET_NEW_TOKEN(data_type::CONST_T, number);
    }

    CLEAR_SPACES;
    return cur_index;
}

int get_name(const Text* data, Tokens* data_store, int cur_str, int cur_index) {
    ASSERT_IF(VALID_PTR(data),       "Invalid data ptr",       0);
    ASSERT_IF(VALID_PTR(data_store), "Invalid data_store ptr", 0);

    char* name = (char*) calloc_s(MAX_NAME_SIZE, sizeof(char));
    int tmp_index = cur_index;

    while (IS_LATIN(DATA) || DATA == '_' || IS_DIGIT(DATA)) {
        if (IS_DIGIT(DATA) && SHIFT == 0) break;
        
        name[SHIFT] = DATA;
        cur_index++;
    }
    name[SHIFT] = '\0';

    if (SHIFT > 0) {
        LOG2(printf("Got a name:   '%s'\n"
                    "              '%s'\n", DATA_STRING, name););
        SET_NEW_TOKEN(data_type::VAR_T, name);
    }

    CLEAR_SPACES;
    return cur_index;
}

int get_operator(const Text* data, Tokens* data_store, int cur_str, int cur_index) {
    ASSERT_IF(VALID_PTR(data),       "Invalid data ptr",       0);
    ASSERT_IF(VALID_PTR(data_store), "Invalid data_store ptr", 0);

    char* name = (char*) calloc_s(MAX_NAME_SIZE, sizeof(char));
    int tmp_index = cur_index;

    while (!IS_DIGIT(DATA) && !IS_LATIN(DATA) && DATA != '_' && DATA != ' ' && DATA != '\0') {
        name[SHIFT] = DATA;
        cur_index++;
    }
    name[SHIFT] = '\0';

    while (SHIFT > 1) {
        if (find_operator(name)) break;

        cur_index--;
        name[SHIFT] = '\0';
    }

    if (SHIFT > 0) {
        LOG2(printf("Got an oper:  '%s'\n"
                    "              '%s'\n", DATA_STRING, name););
        SET_NEW_TOKEN(data_type::OPR_T, name);
    }

    CLEAR_SPACES;
    return cur_index;
}

int find_operator(const char* oper) {
    for (int i = 0; i < OPERATORS_AMOUNT; i++) {
        if (strcmp(oper, OPERATORS[i]) == 0) {
            return 1;
        }
    }

    return 0;
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
        fprintf(log, "Lexem %3d in %d string: type - %d %s\n", i, cur_lex.string, cur_lex.type, cur_lex.type != data_type::ERROR_T ? "" : BAD_OUTPUT);

        fprintf(log, "                       data - " PURPLE);
        if      (cur_lex.type == data_type::CONST_T) fprintf(log, "%d\n", cur_lex.data.number);
        else if (cur_lex.type == data_type::VAR_T ||
                 cur_lex.type == data_type::OPR_T)   fprintf(log, "'%s'\n", cur_lex.data.name);
        else                                         fprintf(log, "%s\n", COLORED_OUTPUT("Unknown data type.\n", RED, log));
        fprintf(log, NATURAL "\n");
    }

    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}
