//
// Created by IvanBrekman on 13.12.2021
//

#ifndef TOKENIZATORH
#define TOKENIZATORH

#include "libs/baselib.h"
#include "libs/file_funcs.h"
#include "libs/tree.h"
#include "src/constants.h"

struct Lexem {
    data_type  type = data_type::ERROR_T;
    data_value data = { };

    int string  = -1;
};

struct Tokens {
    Lexem* data = { };
    int size    = -1;
};

int print_lexem(const Lexem* lex);

Tokens*  get_tokens(const char* filename);
Tokens* take_tokens(const Text* data);

int  get_number  (const Text* data, Tokens* data_store, int* cur_str, int cur_index);
int  get_name    (const Text* data, Tokens* data_store, int* cur_str, int cur_index);
int  get_operator(const Text* data, Tokens* data_store, int* cur_str, int cur_index);
int find_operator(const char* oper);

int tokens_dump(const Tokens* tokens, const char* reason, FILE* log=stdout);

#endif // TOKENIZATORH
