//
// Created by IvanBrekman on 13.12.2021
//

#include <cstring>

#include "config.h"
#include "anal.h"
#include "analDSL.h"

Tree* build_ast_tree(Tokens* tokens, const char* path) {
    ASSERT_IF(VALID_PTR(tokens), "Invalid tokens ptr", NULL);
    ASSERT_IF(VALID_PTR(path),   "Invalid path ptr",   NULL);

    Tree* AST_tree = NEW_PTR(Tree, 1);
    tree_ctor(AST_tree);

    CompileContext* context = NEW_PTR(CompileContext, 1);
    *context = {
        .prs_ctx        = { tokens, 0 },
        .name_table     = {
            .global         = { strdup("global"), NEW_PTR(Name, MAX_NAMES_AMOUNT), 0 },
            .locals         = NEW_PTR(Namespace, MAX_LOCAL_NAMESPACES_AMOUNT),
            .locals_amount  = 0,
        },
        .src_filepath   = path,
        .expected_sym   = NULL,
        .require_state  = NON_REQUIRED,
        .in_function    = 0,
        .require_return = 0
    };
    context->cur_namespace = &context->name_table.global;

    SyntaxContext* result_data = CALL_RULE(GR);
    set_new_root(AST_tree, result_data->node);

    print_name_table(context);

    return AST_tree;
}

NameContext* defined_name(const char* name, CompileContext* context) {
    ASSERT_IF(VALID_PTR(name),    "Invalid name ptr", NULL);
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", NULL);

    NameContext* ctx = NEW_PTR(NameContext, 1);
    *ctx = { name_type::NONE, name_type::NONE, 0 };

    if (!IN_GLOBAL_NAMESPACE) {
        Namespace* cur_namespace = NULL;
        for (int i = 0; i < NAME_TABLE.locals_amount; i++) {
            if (EQUAL(LOCAL_NAMESPACES[i].id, NAMESPACE->id)) {
                cur_namespace = &LOCAL_NAMESPACES[i];
            }
        }
        ASSERT_IF(VALID_PTR(cur_namespace), "Cant get namespace with this name", NULL);

        for (int i = 0; i < cur_namespace->size; i++) {
            Name cur_name = cur_namespace->names[i];
            if (EQUAL(name, cur_name.name)) {
                ctx->local_view  = cur_name.type;
                ctx->is_defined  = 1;
                ctx->args_amount = cur_name.args_amount;
                break;
            }
        }
    }

    for (int i = 0; i < GLOBAL_NAMESPACE.size; i++) {
        Name cur_name = GLOBAL_NAMESPACE.names[i];
        if (EQUAL(name, cur_name.name)) {
            ctx->global_view = cur_name.type;
            ctx->is_defined = 1;
            ctx->args_amount = cur_name.args_amount;
            break;
        }
    }

    return ctx;
}

int check_redefinition(const char* name, CompileContext* context) {
    ASSERT_IF(VALID_PTR(name),    "Invalid name ptr",    -1);
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", -1);

    NameContext* ctx = defined_name(name, context);
    LOGN(3, printf("name: '%s', ctx local: %d, ctx global: %d\n", name, ctx->local_view, ctx->global_view););

    if (IN_GLOBAL_NAMESPACE) return ctx->global_view != name_type::NONE;
    return ctx->local_view != name_type::NONE;
}

int check_type(const char* name, CompileContext* context, name_type type) {
    ASSERT_IF(VALID_PTR(name),    "Invalid name ptr",    -1);
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", -1);

    NameContext* ctx = defined_name(name, context);
    LOGN(3, printf("name: '%s', ctx local: %d, ctx global: %d\n", name, ctx->local_view, ctx->global_view););

    if (IN_GLOBAL_NAMESPACE)                return ctx->global_view == type;
    if (ctx->local_view != name_type::NONE) return ctx->local_view  == type;

    return ctx->global_view == type;
}

int print_name_table(const CompileContext* context) {
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", -1);

    printf("NameTable-------------------\n");

    print_namespace(&GLOBAL_NAMESPACE);
    for (int i = 0; i < NAME_TABLE.locals_amount; i++) {
        print_namespace(&LOCAL_NAMESPACES[i]);
    }
    printf("----------------------------\n");

    return 1;
}

int print_namespace(const Namespace* namespace_) {
    ASSERT_IF(VALID_PTR(namespace_), "Invalid namespace_ ptr", -1);

    printf("Namespace '%s':\n", namespace_->id);

    for (int i = 0; i < namespace_->size; i++) {
        Name cur_name = namespace_->names[i];
        printf("    Type: %s; Name: '%s'\n", cur_name.type == name_type::VARIABLE ? " VAR" : "FUNC", cur_name.name);
    }

    return 1;
}

int is_reserved_name(const char* name) {
    ASSERT_IF(VALID_PTR(name), "Invalid name ptr", -1);

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
    if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected Statement");

    Node* cur_node = func_ctx->node;
    SyntaxContext* st = NULL;
    while (st = CALL_RULE(Statement)) {
        add_child(cur_node, st->node, child_type::RIGHT);
        cur_node = st->node;
    }

    RETURN_WITH_REQUIRE(FINISH_TOKEN);
}

GRAMMAR_RULE(Statement) {
    INIT;

    if (EQUAL_LEXEM(data_type::OPR_T, END_STATEMENT)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));

    if (EQUAL_LEXEM(data_type::OPR_T, START_BLOCK)) {
        TOKENS_PTR++;
        if (EQUAL_LEXEM(data_type::OPR_T, END_BLOCK)) RETURN_WITH_REBIND(END_BLOCK, data_type::OPR_T, strdup(";"));

        REQUIRE_RETURN = 0;

        func_ctx = CALL_RULE(Statement);
        if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected Statement after START_BLOCK symbol");

        Node* cur_node = func_ctx->node;
        SyntaxContext* st = NULL;
        while (st = CALL_RULE(Statement)) {
            add_child(cur_node, st->node, child_type::RIGHT);
            cur_node = st->node;
        }

        RETURN_WITH_REQUIRE(END_BLOCK);
    }

    if (IN_FUNCTION) {
        func_ctx = CALL_RULE(Return);
        if (RULE_DONE(func_ctx)) {
            REBIND_CNT_LEFT(data_type::OPR_T, strdup(";"));
            RETURN_COMPLETED;
        }
        if (REQUIRE_RETURN) THROW_ERROR("Expected return in function");
    }
    
    func_ctx = CALL_RULE(If);
    if (RULE_DONE(func_ctx)) {
        REBIND_CNT_LEFT(data_type::OPR_T, strdup(";"));
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(While);
    if (RULE_DONE(func_ctx)) {
        REBIND_CNT_LEFT(data_type::OPR_T, strdup(";"));
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Fdef);
    if (RULE_DONE(func_ctx)) {
        REBIND_CNT_LEFT(data_type::OPR_T, strdup(";"));
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Vdef);
    if (RULE_DONE(func_ctx)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));

    func_ctx = CALL_RULE(Exp);
    if (RULE_DONE(func_ctx)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"));

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(While) {
    INIT;

    if (EQUAL_LEXEM(data_type::VAR_T, "while")) {
        TOKENS_PTR++;

        func_ctx = CALL_RULE(If_cond);
        if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected Statement after while ()");

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
        if (!RULE_DONE(st)) THROW_ERROR("Expected Statement after else");

        add_child(cur_node, st->node, child_type::RIGHT);
    }

    RETURN_COMPLETED;
}

GRAMMAR_RULE(If_cond) {
    INIT;

    // Try get '(exp)'
    if (!EQUAL_LEXEM(data_type::OPR_T, OPEN_BRACKET)) THROW_ERROR("Expected OPEN_BRACKET after if");
    TOKENS_PTR++;

    func_ctx = CALL_RULE(Exp);
    if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected expression in if");

    if (!EQUAL_LEXEM(data_type::OPR_T, CLOSE_BRACKET)) THROW_ERROR("Expected CLOSE_BRACKET after expression");
    TOKENS_PTR++;
    //

    // Try get 'Statement'
    SyntaxContext* st = CALL_RULE(Statement);
    if (!RULE_DONE(st)) THROW_ERROR("Expected statement after if");

    REBIND_CNT_LEFT(data_type::OPR_T, strdup("if"));
    REBIND_NODE(data_type::OPR_T, strdup("if_else"), st, child_type::LEFT);
    add_child(func_ctx->node, st->node, child_type::RIGHT);
    //

    RETURN_COMPLETED;
}

GRAMMAR_RULE(Fdef) {
    INIT;

    if (!EQUAL_LEXEM(data_type::VAR_T, "func")) RETURN_NOT_COMPLETED;
    TOKENS_PTR++;

    func_ctx = CALL_RULE(Id);
    if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected func name");
    CHECK_REDEFINITION(MAIN_NAME);

    IN_FUNCTION    = 1;
    REQUIRE_RETURN = 1;
    ADD_NAMESPACE(func_ctx->main_name);
    ADD_NAME(name_type::FUNCTION, MAIN_NAME);

    NAMESPACE = &LOCAL_NAMESPACES[NAME_TABLE.locals_amount - 1];
    ADD_NAME(name_type::FUNCTION, MAIN_NAME);
    REBIND_CNT_LEFT(data_type::VAR_T, strdup("func"));

    HARD_REQUIRE(OPEN_BRACKET);

    int args_amount = 0;
    Node* cur_node = func_ctx->node->left;
    SyntaxContext* args = NULL;
    while (args = CALL_RULE(Id)) {
        CHECK_REDEFINITION(args->main_name);
        ADD_NAME(name_type::VARIABLE, args->main_name);

        add_child(cur_node, args->node, child_type::LEFT);
        cur_node = args->node;
        args_amount++;

        if (!EQUAL_LEXEM(data_type::OPR_T, ",")) break;
        TOKENS_PTR++;
    }
    HARD_REQUIRE(CLOSE_BRACKET);

    LAST_NAME(GLOBAL_NAMESPACE).args_amount                               = args_amount;
    LAST_NAME(LOCAL_NAMESPACES[NAME_TABLE.locals_amount - 1]).args_amount = args_amount;

    SyntaxContext* st = CALL_RULE(Statement);
    if (!RULE_DONE(st)) THROW_ERROR("Expected statement after func definition");
    add_child(func_ctx->node, st->node, child_type::RIGHT);

    IN_FUNCTION    = 0;
    REQUIRE_RETURN = 0;

    NAMESPACE   = &GLOBAL_NAMESPACE;
    RETURN_COMPLETED;
}

GRAMMAR_RULE(Return) {
    INIT;

    if (!EQUAL_LEXEM(data_type::VAR_T, "return")) RETURN_NOT_COMPLETED;
    TOKENS_PTR++;

    func_ctx = CALL_RULE(Exp);
    if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected Expression after return");

    RETURN_WITH_REBIND(END_STATEMENT, data_type::VAR_T, strdup("return"));
}

GRAMMAR_RULE(Call) {
    INIT;

    func_ctx = CALL_RULE(Id);
    if (!RULE_DONE(func_ctx)) RETURN_NOT_COMPLETED;
    if (!IS_FUNCTION(MAIN_NAME)) {
        TOKENS_PTR--;
        RETURN_NOT_COMPLETED;
    }

    HARD_REQUIRE(OPEN_BRACKET);
    REBIND_CNT_LEFT(data_type::VAR_T, strdup("call"));

    int args_amount = defined_name(MAIN_NAME, context)->args_amount;

    int real_args_am = 0;
    Node* cur_node = func_ctx->node;
    SyntaxContext* args = NULL;
    while (args = CALL_RULE(Exp)) {
        REBIND_NODE(data_type::OPR_T, strdup(";"), args, LEFT);
        add_child(cur_node, args->node, child_type::RIGHT);
        cur_node = args->node;
        real_args_am++;

        if (real_args_am > args_amount) THROW_ERROR("Too many agruments for function");
        if (!EQUAL_LEXEM(data_type::OPR_T, ",")) break;
        TOKENS_PTR++;
    }

   if (real_args_am < args_amount) THROW_ERROR("Not enough arguments for function");

    HARD_REQUIRE(CLOSE_BRACKET);
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
        CHECK_REDEFINITION(MAIN_NAME);
        ADD_NAME(name_type::VARIABLE, MAIN_NAME);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Id);
    if (RULE_DONE(func_ctx)) {
        CHECK_REDEFINITION(MAIN_NAME);
        ADD_NAME(name_type::VARIABLE, MAIN_NAME);
        RETURN_COMPLETED;
    }
    
    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(Ass) {
    INIT;

    SyntaxContext* variable   = CALL_RULE(Id);
    if (!RULE_DONE(variable)) RETURN_NOT_COMPLETED;

    if (!SOFT_REQUIRE("=") || IS_FUNCTION(variable->main_name)) {
        dbg();
        TOKENS_PTR--;
        RETURN_NOT_COMPLETED;
    }

    SyntaxContext* expression = CALL_RULE(Exp);
    if (!RULE_DONE(expression)) THROW_ERROR("Expected Expression in Assignment");

    SET_NODE_NAME(data_type::OPR_T, strdup("="));
    SET_MAIN_NAME(variable->main_name);

    add_child(func_ctx->node, variable->node,   child_type::LEFT);
    add_child(func_ctx->node, expression->node, child_type::RIGHT);

    RETURN_COMPLETED;
}

GRAMMAR_RULE(Exp) {
    INIT;

    func_ctx = CALL_RULE(Ass);
    if (RULE_DONE(func_ctx)) {
        CHECK_UNDEFINITION(MAIN_NAME);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Cmp);
    if (RULE_DONE(func_ctx)) RETURN_COMPLETED;

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(Cmp) {
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

    if (EQUAL_LEXEM(data_type::OPR_T, OPEN_BRACKET)) {
        TOKENS_PTR++;

        func_ctx = CALL_RULE(Exp);
        if (!RULE_DONE(func_ctx)) THROW_ERROR("Expexted expression after OPEN_BRACKET");

        if (unary_oper) RETURN_WITH_REBIND(CLOSE_BRACKET, data_type::OPR_T, unary_oper);
        RETURN_WITH_REQUIRE(CLOSE_BRACKET);
    }

    func_ctx = CALL_RULE(Number);
    if (RULE_DONE(func_ctx)) {
        if (unary_oper) REBIND_CNT_LEFT(data_type::OPR_T, unary_oper);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Call);
    if (RULE_DONE(func_ctx)) {
        if (unary_oper) REBIND_CNT_LEFT(data_type::OPR_T, unary_oper);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Id);
    if (RULE_DONE(func_ctx)) {
        CHECK_UNDEFINITION(MAIN_NAME);
        if (!IS_VARIABLE(MAIN_NAME)) {
            TOKENS_PTR--;
            THROW_ERROR("Not a Variable");
        }

        if (unary_oper) REBIND_CNT_LEFT(data_type::OPR_T, unary_oper);
        RETURN_COMPLETED;
    }

    if (unary_oper) THROW_ERROR("Expected Expression after unary operator");
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
    if (!IS_NUMBER) printf("name: '%s'\n", LEXEM_NAME);
    if (!IS_IDENTIFIER || is_reserved_name(LEXEM_NAME)) {
        RETURN_NOT_COMPLETED;
    }

    SET_NODE_NAME(data_type::VAR_T, LEXEM_NAME);
    SET_MAIN_NAME(LEXEM_NAME);

    TOKENS_PTR++;
    RETURN_COMPLETED;
}

int Require(const char* string, CompileContext* context) {
    ASSERT_IF(VALID_PTR(string),  "Invalid string ptr",   0);
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr",  0);

    if (IS_NUMBER || !EQUAL(LEXEM_NAME, string)) {
        context->expected_sym = string;
        if (context->require_state == REQUIRED) THROW_ERROR("Expected required string");
        context->expected_sym = NULL;
        return 0;
    }

    TOKENS_PTR++;
    return 1;
}

int SyntaxError(const char* description, const CompileContext* context) {
    ASSERT_IF(VALID_PTR(description), "Invalid description ptr", -1);
    ASSERT_IF(VALID_PTR(context),     "Invalid context ptr",     -1);

    print_name_table(context);

    printf(RED "SyntaxError\n" NATURAL);
    if (VALID_PTR(context->expected_sym)) printf("%s " PURPLE "'%s'" NATURAL, description, context->expected_sym);
    else                                  printf("%s", description);

    printf(", got " PURPLE "'");
    print_lexem(&LEXEM);
    printf("'" NATURAL ".\n%s:%d ->   ", FILEPATH, LEXEM.string);

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
