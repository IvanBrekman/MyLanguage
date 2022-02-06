//
// Created by IvanBrekman on 13.12.2021
//

#ifndef ANALIZATORH
#define ANALIZATORH

#include <limits.h>

#include "libs/baselib.h"
#include "libs/tree.h"
#include "src/constants.h"

#include "../Tokenizer/token.h"

enum name_type {
    NONE     = 0,

    VARIABLE = 1,
    FUNCTION = 2
};

struct Name {
    char*     name  = NULL;
    name_type type  = name_type::NONE;

    int args_amount = -1;
    int address     = -1;
};

struct Namespace {
    char* id    = NULL;
    Name* names = NULL;
    int   size  = -1;
};

struct NameTable {
    Namespace  global = { };
    Namespace* locals = NULL;

    int locals_amount = -1;
};

struct StdContext {
    int*             used_functions = NULL;
    int func_amount                 = -1;
};

struct FrontContext {
    Tree*       AST_tree  = NULL;
    NameTable*  nametable = NULL;
    StdContext* std_ctx   = NULL;
};

struct ParseContext {
    Tokens* tokens = NULL;
    int pointer    = -1;
};

struct NameContext {
    name_type global_view = name_type::NONE;
    name_type  local_view = name_type::NONE;

    int is_defined  = 0;
    int args_amount = 0;
};

struct SyntaxContext {
    Node* node      = NULL;
    char* main_name = NULL;

    int token_shift = 0;
};

struct CompileContext {
    ParseContext prs_ctx = { };
    NameTable  nametable = { };
    StdContext std_ctx   = { };
    Namespace*   cur_namespace = NULL;

    const char* src_filepath  = NULL;
    const char* expected_sym  = NULL;

    int require_state  = NON_REQUIRED;
    int in_function    =  0;
    int require_return =  0;
};

struct ASMGenerateContext {
    FILE*       asm_file      = NULL;

    NameTable*  nametable     = NULL;
    Namespace*  cur_namespace = NULL;

    StdContext* std_ctx       = NULL;

    int         asm_indent    = -1;
    int         asm_length    = -1;
};

struct StandardFunction {
    const char* name = NULL;
    int         code = -1;
    int (*execute_func) (ASMGenerateContext* asm_file);

    int  min_args = 0;
    int  max_args = INT_MAX;
    int real_args = 0;
};

FrontContext* build_ast_tree(Tokens* tokens, const char* path);
NameContext* defined_name(const char* name, CompileContext* context);
int    check_redefinition(const char* name, CompileContext* context);
int check_type(const char* name, CompileContext* context, name_type type);

int print_nametable (const NameTable* nametable);
int print_namespace  (const Namespace* namespace_);
int print_std_context(      StdContext* std_ctx);
int is_reserved_name (const char* name);
int is_std_name(CompileContext* context);

// RULES-----------------------------------------------------------------------
/*
    Gr          ::= {Statement}+ '$'
    Statement   ::= '{' Statement +? Return '}' | ?Return | If | While | For | Fdef | {Vdef | Exp} ';'

    Fdef        ::= 'func' Id '(' {Id ',' }* ')' (Statement + Return)
    Return      ::= 'return' Exp ';'
    Call        ::= Id '(' {Exp ',' }* ')'
    StdFunc     ::= Id '(' {Exp ',' }* ')'

    For         ::= 'for' '(' {Vdef | Ass} '->' Exp { ':' Exp}? ')' Statement

    While       ::= 'while' If_cond
    If          ::= 'if' If_cond { 'elif' If_cond}* { 'else' Statement}?
    If_cond     ::= '(' Exp ')' Statement

    Vdef        ::= 'def' Ass
    Ass         ::= Id '=' Exp
    Exp         ::= Ass | Cmp
    Cmp         ::= Lvalue{[< > <= >= == !=]Lvalue}*

    Lvalue      ::= T{[+-]T}*
    T           ::= P{[/*%]P}*
    P           ::= {-+!}? '(' Exp ')' | Number | Call | StdFunc | Id

    Number      ::= [0-9]
    Id          ::= [a-zA-Z_]{[a-zA-Z_0-9]}*
*/

#define GRAMMAR_RULE(name)      SyntaxContext* Rule_ ## name(CompileContext* context)
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
        SET_NODE_NAME(data_type::OPR_T, LEXEM_NAME, func_ctx);                          \
        NODE_SAVING_STATE = 1;                                                          \
        TOKENS_PTR++;                                                                   \
                                                                                        \
        SyntaxContext* number = CALL_RULE(called_ryle);                                 \
        if (!RULE_DONE(number)) THROW_ERROR("Expexted expression part after operator"); \
                                                                                        \
        add_child(func_ctx->node, tmp,          child_type::LEFT );                     \
        add_child(func_ctx->node, number->node, child_type::RIGHT);                     \
    }                                                                                   \
}

GRAMMAR_RULE(GR);
GRAMMAR_RULE(Statement);

GRAMMAR_RULE(Fdef);
GRAMMAR_RULE(Return);
GRAMMAR_RULE(Call);
GRAMMAR_RULE(StdFunc);

GRAMMAR_RULE(For);

GRAMMAR_RULE(While);
GRAMMAR_RULE(If);
GRAMMAR_RULE(If_cond);

GRAMMAR_RULE(Vdef);
GRAMMAR_RULE(Ass);
GRAMMAR_RULE(Exp);
GRAMMAR_RULE(Cmp);

GRAMMAR_RULE(Lvalue);
GRAMMAR_RULE(T);
GRAMMAR_RULE(P);

GRAMMAR_RULE(Number);
GRAMMAR_RULE(Id);
// ----------------------------------------------------------------------------

int     Require(const char* string,            CompileContext* context);
int SyntaxError(const char* description, const CompileContext* context);

#endif // ANALIZATORH
