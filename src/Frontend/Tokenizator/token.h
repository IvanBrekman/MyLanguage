//
// Created by IvanBrekman on 13.12.2021
//

#ifndef TOKENIZATORH
#define TOKENIZATORH

#include "../../../libs/baselib.h"
#include "../../../libs/tree.h"

union token_data {
    int number = poisons::UNINITIALIZED_INT;
    char* name;
};

struct Lexem {
    data_type  type = data_type::ERROR_T;
    token_data data = { };

    Node* left  = NULL;
    Node* right = NULL;
};

struct Tokens {
    Lexem* data = { };
    int size    = -1;
};

Tokens* get_tokens(const char* filename);
Tokens* take_tokens(const char* data);

int get_number  (const char* data, Tokens* data_store);
int get_name    (const char* data, Tokens* data_store);
int get_operator(const char* data, Tokens* data_store);

int tokens_dump(const Tokens* tokens, const char* reason, FILE* log=stdout);

#endif // TOKENIZATORH
