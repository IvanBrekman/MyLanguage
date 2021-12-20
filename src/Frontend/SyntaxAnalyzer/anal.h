//
// Created by IvanBrekman on 13.12.2021
//

#ifndef ANALIZATORH
#define ANALIZATORH

#include "libs/baselib.h"
#include "libs/tree.h"
#include "src/constants.h"

#include "analDSL.h"
#include "../Tokenizer/token.h"

struct ParseContext {
    Tokens* tokens = NULL;
    int pointer    = -1;
};

enum name_type {
    NONE     = 0,

    VARIABLE = 1,
    FUNCTION = 2
};

struct Name {
    char*     name = NULL;
    name_type type = name_type::NONE;
};

struct NameTable {
    Name* names = NULL;
    int   size  = -1;
};

struct SyntaxContext {
    Node* node      = NULL;
    char* main_name = NULL;

    int token_shift = 0;
};

Tree* build_ast_tree(Tokens* tokens, const char* path);
int print_name_table();

// RULES-----------------------------------------------------------------------
/*
    Gr          ::= {Statement}+ '$'
    Statement   ::= '{'Statement'}' | If | While | {Vdef | Ass | Exp};
    While       ::= 'while' If_cond
    If          ::= 'if' If_cond {'elif' If_cond}* {'else' Statement}?
    If_cond     ::= '('Exp')' Statement
    Vdef        ::= 'def' {Ass | Id}
    Ass         ::= Id'='Exp
    Exp         ::= Lvalue{[< > <= >= == !=]Lvalue}*
    Lvalue      ::= T{[+-]T}*
    T           ::= P{[/*]P}*
    P           ::= {-}? '('Exp')' | Number | Id
    Number      ::= [0-9]
    Id          ::= [a-zA-Z_]{[a-zA-Z_0-9]}*
*/

#define EXPRESSION_PRIORITY_RULE(called_ryle, _operators...) {                          \
    func_ctx = CALL_RULE(called_ryle);                                                  \
    if (!RULE_DONE(func_ctx)) RETURN_NOT_COMPLETED;                                     \
                                                                                        \
    const char* operators[] = { _operators };                                           \
    while (1) {                                                                         \
        int can_continue = 0;                                                           \
        for  (int i = 0; i < sizeof(operators) / sizeof(operators[0]); i++) {           \
            if (EQUAL_LEXEM(data_type::OPR_T, operators[i])) can_continue = 1;          \
        }                                                                               \
        if (!can_continue) break;                                                       \
                                                                                        \
        Node* tmp = func_ctx->node;                                                     \
        SET_NODE_NAME(data_type::OPR_T, LEXEM_NAME);                                    \
        TOKENS_PTR++;                                                                   \
                                                                                        \
        SyntaxContext* number = CALL_RULE(called_ryle);                                 \
        if (!RULE_DONE(number)) SyntaxError("Expexted expression part after operator"); \
                                                                                        \
        add_child(func_ctx->node, tmp, child_type::LEFT);                               \
        add_child(func_ctx->node, number->node, child_type::RIGHT);                     \
    }                                                                                   \
}

GRAMMAR_RULE(GR);
GRAMMAR_RULE(Statement);

GRAMMAR_RULE(While);
GRAMMAR_RULE(If);
GRAMMAR_RULE(If_cond);

GRAMMAR_RULE(Vdef);
GRAMMAR_RULE(Ass);

GRAMMAR_RULE(Exp);
GRAMMAR_RULE(Lvalue);
GRAMMAR_RULE(T);
GRAMMAR_RULE(P);

GRAMMAR_RULE(Number);
GRAMMAR_RULE(Id);
// ----------------------------------------------------------------------------

int Require(const char* string, int require_state, SyntaxContext* func_ctx);
int SyntaxError(const char* description, const char* exp_sym=NULL);

#endif // ANALIZATORH
