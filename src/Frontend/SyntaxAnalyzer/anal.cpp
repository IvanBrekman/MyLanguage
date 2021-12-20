//
// Created by IvanBrekman on 13.12.2021
//

#include <cstring>

#include "config.h"
#include "anal.h"
#include "analDSL.h"

ParseContext context = { };
NameTable name_table = { (Name*) calloc_s(MAX_NAMES_AMOUNT, sizeof(Name)), 0 };
const char* filename = NULL;

Tree* build_ast_tree(Tokens* tokens, const char* path) {
    ASSERT_IF(VALID_PTR(tokens), "Invalid tokens ptr", NULL);

    filename = path;

    Tree* AST_tree = NEW_PTR(Tree);
    tree_ctor(AST_tree);

    TOKENS     = tokens;
    TOKENS_PTR = 0;

    int require_state = REQUIRED;
    SyntaxContext* result_data = CALL_RULE(GR);
    set_new_root(AST_tree, result_data->node);

    print_name_table();

    return AST_tree;
}

int print_name_table() {
    printf("NameTable---------\n");

    for (int i = 0; i < name_table.size; i++) {
        Name cur_name = name_table.names[i];
        printf("Type: %s; Name: '%s'\n", cur_name.type == VARIABLE ? " VAR" : "FUNC", cur_name.name);
    }
    printf("------------------\n");

    return 1;
}

name_type defined_name(const char* name) {
    ASSERT_IF(VALID_PTR(name), "Invalid name ptr", name_type::NONE);

    for (int i = 0; i < name_table.size; i++) {
        Name cur_name = name_table.names[i];
        if (EQUAL(name, cur_name.name)) {
            return cur_name.type;
        }
    }

    return name_type::NONE;
}

int is_reserved_name(const char* name) {
    for (int i = 0; i < RESERVED_NAMES_AMOUNT; i++) {
        if (strcmp(name, RESERVED_NAMES[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

// RULES-----------------------------------------------------------------------
GRAMMAR_RULE(GR) {
    INIT;

    func_ctx = CALL_RULE(Statement);
    if (!RULE_DONE(func_ctx)) SyntaxError("Expected Statement");
        printf("tokens_ptr %d\n", TOKENS_PTR);

    Node* cur_node = func_ctx->node;
    SyntaxContext* st = NULL;
    printf("tokens_ptr1 %d\n", TOKENS_PTR);
    while (st = CALL_RULE(Statement)) {
        printf("tokens_ptr cycle %d\n", TOKENS_PTR);
        add_child(cur_node, st->node, child_type::RIGHT);
        cur_node = st->node;
    }
    printf("tokens_ptr2 %d\n", TOKENS_PTR);

    RETURN_WITH_REQUIRE(FINISH_TOKEN);
}

GRAMMAR_RULE(Statement) {
    INIT;

    if (EQUAL_LEXEM(data_type::OPR_T, END_STATEMENT)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));

    if (EQUAL_LEXEM(data_type::OPR_T, START_BLOCK)) {
        TOKENS_PTR++;
        if (EQUAL_LEXEM(data_type::OPR_T, END_BLOCK)) RETURN_WITH_REBIND(END_BLOCK, data_type::OPR_T, strdup(";"));

        func_ctx = CALL_RULE(Statement);
        if (!RULE_DONE(func_ctx)) SyntaxError("Expected Statement after START_BLOCK symbol");

        Node* cur_node = func_ctx->node;
        SyntaxContext* st = NULL;
        while (st = CALL_RULE(Statement)) {
            add_child(cur_node, st->node, child_type::RIGHT);
            cur_node = st->node;
        }

        RETURN_WITH_REQUIRE(END_BLOCK);
    }
    
    func_ctx = CALL_RULE(If);
    if (RULE_DONE(func_ctx)) {
        REBIND_CNT_LEFT(data_type::OPR_T, strdup(";"));
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(While);
    if (RULE_DONE(func_ctx)) {
        REBIND_CNT_LEFT(data_type::OPR_T, strdup(";"));
        printf("tokens_ptr %d\n", TOKENS_PTR);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Vdef);
    if (RULE_DONE(func_ctx)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));

    func_ctx = CALL_RULE(Ass);
    if (RULE_DONE(func_ctx)) {
        CHECK_UNDEFINITION("Variable");
        RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));
    }

    func_ctx = CALL_RULE(Exp);
    if (RULE_DONE(func_ctx)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(While) {
    INIT;

    if (EQUAL_LEXEM(data_type::VAR_T, "while")) {
        TOKENS_PTR++;

        func_ctx = CALL_RULE(If_cond);
        if (!RULE_DONE(func_ctx)) SyntaxError("Expected Statement after while ()");

        func_ctx->node->data.value.name = strdup("while");
        RETURN_COMPLETED;
    }

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(If) {
    INIT;

    if (!EQUAL_LEXEM(data_type::VAR_T, "if")) RETURN_NOT_COMPLETED;
    TOKENS_PTR++;
    
    func_ctx = CALL_RULE(If_cond);
    if (!RULE_DONE(func_ctx)) RETURN_NOT_COMPLETED;

    Node* cur_node = func_ctx->node->right;
    ASSERT_IF(VALID_PTR(cur_node), "Invalid cur_node ptr", NULL);

    while (1) {
        if (!EQUAL_LEXEM(data_type::VAR_T, "elif")) break;
        TOKENS_PTR++;

        SyntaxContext* if_base = CALL_RULE(If_cond);
        if (!RULE_DONE(if_base)) break;
        
        add_child(cur_node, if_base->node, child_type::RIGHT);
        cur_node = if_base->node->right;
        ASSERT_IF(VALID_PTR(cur_node), "Invalid cur_node ptr in cycle", NULL);
    }

    if (EQUAL_LEXEM(data_type::VAR_T, "else")) {
        TOKENS_PTR++;

        SyntaxContext* st = CALL_RULE(Statement);
        if (!RULE_DONE(st)) SyntaxError("Expected Statement after else");

        add_child(cur_node, st->node, child_type::RIGHT);
    }

    RETURN_COMPLETED;
}

GRAMMAR_RULE(If_cond) {
    INIT;

    // Try get '(exp)'
    if (!EQUAL_LEXEM(data_type::OPR_T, OPEN_BRACKET)) SyntaxError("Expected OPEN_BRACKET after if");
    TOKENS_PTR++;

    func_ctx = CALL_RULE(Exp);
    if (!RULE_DONE(func_ctx)) SyntaxError("Expected expression in if");

    if (!EQUAL_LEXEM(data_type::OPR_T, CLOSE_BRACKET)) SyntaxError("Expected CLOSE_BRACKET after expression");
    TOKENS_PTR++;
    //

    // Try get 'Statement'
    SyntaxContext* st = CALL_RULE(Statement);
    if (!RULE_DONE(st)) SyntaxError("Expected statement after if");

    REBIND_CNT_LEFT(data_type::OPR_T, strdup("if"));
    REBIND_NODE(data_type::OPR_T, strdup("if_else"), st, child_type::LEFT);
    add_child(func_ctx->node, st->node, child_type::RIGHT);
    //

    RETURN_COMPLETED;
}

GRAMMAR_RULE(Vdef) {
    INIT;

    if (!EQUAL_LEXEM(data_type::VAR_T, "def")) {
        RETURN_NOT_COMPLETED;
    }
    TOKENS_PTR++;

    func_ctx = CALL_RULE(Ass);
    if (RULE_DONE(func_ctx)) {
        CHECK_REDEFINITION("Variable");
        ADD_NAME(name_type::VARIABLE);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Id);
    if (RULE_DONE(func_ctx)) {
        CHECK_REDEFINITION("Variable");
        ADD_NAME(name_type::VARIABLE);
        // func_ctx->node = NULL;
        RETURN_COMPLETED;
    }
    
    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(Ass) {
    INIT;

    SyntaxContext* variable   = CALL_RULE(Id);
    if (!RULE_DONE(variable)) RETURN_NOT_COMPLETED;

    if (!SOFT_REQUIRE("=")) {
        TOKENS_PTR--;
        RETURN_NOT_COMPLETED;
    }

    SyntaxContext* expression = CALL_RULE(Exp);
    if (!RULE_DONE(expression)) SyntaxError("Expected Expression in Assignment");

    SET_NODE_NAME(data_type::OPR_T, strdup("="));
    SET_MAIN_NAME(variable->main_name);

    add_child(func_ctx->node, variable->node,   child_type::LEFT);
    add_child(func_ctx->node, expression->node, child_type::RIGHT);

    RETURN_COMPLETED;
}

GRAMMAR_RULE(Exp) {
    INIT;

    EXPRESSION_PRIORITY_RULE(Lvalue, "<", ">", "<=", ">=", "==", "!=");

    RETURN_COMPLETED;
}

GRAMMAR_RULE(Lvalue) {
    INIT;

    EXPRESSION_PRIORITY_RULE(T, "+", "-");

    RETURN_COMPLETED;
}

GRAMMAR_RULE(T) {
    INIT;

    EXPRESSION_PRIORITY_RULE(P, "*", "/");

    RETURN_COMPLETED;
}

GRAMMAR_RULE(P) {
    INIT;

    char* unary_oper = NULL;
    if (EQUAL_LEXEM(data_type::OPR_T, "-")) {
        TOKENS_PTR++;
        unary_oper = strdup("-");
    }

    if (EQUAL_LEXEM(data_type::VAR_T, OPEN_BRACKET)) {
        TOKENS_PTR++;

        func_ctx = CALL_RULE(Exp);
        if (!RULE_DONE(func_ctx)) SyntaxError("Expexted expression after OPEN_BRACKET");

        if (unary_oper) RETURN_WITH_REBIND(CLOSE_BRACKET, data_type::OPR_T, unary_oper);
        RETURN_WITH_REQUIRE(CLOSE_BRACKET);
    }

    func_ctx = CALL_RULE(Number);
    if (RULE_DONE(func_ctx)) {
        if (unary_oper) REBIND_CNT_LEFT(data_type::OPR_T, unary_oper);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Id);
    if (RULE_DONE(func_ctx)) {
        CHECK_UNDEFINITION("Variable");
        if (unary_oper) REBIND_CNT_LEFT(data_type::OPR_T, unary_oper);
        RETURN_COMPLETED;
    }

    if (unary_oper) SyntaxError("Expected Expression after unary operator");
    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(Number) {
    INIT;

    if (!IS_NUMBER) {
        RETURN_NOT_COMPLETED;
    }

    SET_NODE_NUMBER(data_type::CONST_T, LEXEM_NUM);

    TOKENS_PTR++;
    RETURN_COMPLETED;
}

GRAMMAR_RULE(Id) {
    INIT;

    if (!IS_IDENTIFIER || is_reserved_name(LEXEM_NAME)) {
        RETURN_NOT_COMPLETED;
    }

    SET_NODE_NAME(data_type::VAR_T, LEXEM_NAME);
    SET_MAIN_NAME(LEXEM_NAME);

    TOKENS_PTR++;
    RETURN_COMPLETED;
}

int Require(const char* string, int require_state, SyntaxContext* func_ctx) {
    ASSERT_IF(VALID_PTR(string),   "Invalid string ptr",   0);
    ASSERT_IF(VALID_PTR(func_ctx), "Invalid func_ctx ptr", 0);

    tokens_dump(context.tokens, "Check before require");
        printf("tokens_ptr3 %d. lexem: '%s' - string: '%s'\n", TOKENS_PTR, LEXEM_NAME, string);
    if (IS_NUMBER || !EQUAL(LEXEM_NAME, string)) {
        if (require_state == REQUIRED) SyntaxError("Expected required string", string);
        return 0;
    }

    TOKENS_PTR++;
    return 1;
}

int SyntaxError(const char* description, const char* exp_sym) {
    ASSERT_IF(VALID_PTR(description), "Invalid description ptr", -1);

    printf(RED "SyntaxError\n" NATURAL);
    if (VALID_PTR(exp_sym)) printf("%s " PURPLE "'%s'" NATURAL, description, exp_sym);
    else                    printf("%s", description);

    printf(", got " PURPLE "'");
    print_lexem(&LEXEM);
    printf("'" NATURAL ".\n%s:%d ->   ", filename, LEXEM.string);

    for (int i = 0; i < TOKENS_SIZE; i++) {
        if (TOKENS->data[i].string != LEXEM.string) continue;

        if (i == TOKENS_PTR) printf(RED);
        print_lexem(&TOKENS->data[i]);
        if (i == TOKENS_PTR) printf(NATURAL);

        if (i + 1 < TOKENS_SIZE) printf(" ");
    }
    printf("\n\n");

    assert(0 && "Intentional break");
    abort();

    return -1;
}
// ----------------------------------------------------------------------------
