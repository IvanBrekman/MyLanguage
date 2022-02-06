//
// Created by IvanBrekman on 13.12.2021
//

#include <cstring>

#include "config.h"
#include "anal.h"
#include "analDSL.h"
#include "src/StandardLibrary/functions.h"

FrontContext* build_ast_tree(Tokens* tokens, const char* path) {
    ASSERT_IF(VALID_PTR(tokens), "Invalid tokens ptr", NULL);
    ASSERT_IF(VALID_PTR(path),   "Invalid path ptr",   NULL);

    FrontContext* front = NEW_PTR(FrontContext, 1);
    front->AST_tree = NEW_PTR(Tree, 1);
    tree_ctor(front->AST_tree);

    CompileContext* context = NEW_PTR(CompileContext, 1);
    *context = {
        .prs_ctx        = { tokens, 0 },
        .nametable      = {
            .global         = { strdup("global"), NEW_PTR(Name, MAX_NAMES_AMOUNT), 0 },
            .locals         = NEW_PTR(Namespace, MAX_LOCAL_NAMESPACES_AMOUNT),
            .locals_amount  = 0,
        },
        .std_ctx        = { NEW_PTR(int, STD_FUNC_AMOUNT), 0 },
        .src_filepath   = path,
        .expected_sym   = NULL,
        .require_state  = NON_REQUIRED,
        .in_function    = 0,
        .require_return = 0,
    };
    context->cur_namespace = &context->nametable.global;

    SyntaxContext* result_data = CALL_RULE(GR);
    set_new_root(front->AST_tree , result_data->node);

    LOG2(
        print_nametable(&NAME_TABLE);
        printf("\n");
        print_std_context(&STD_CTX);
    );
    front->nametable = &context->nametable;
    front->std_ctx   = &context->std_ctx;

    return front;
}

NameContext* defined_name(const char* name, CompileContext* context) {
    ASSERT_IF(VALID_PTR(name),    "Invalid name ptr", NULL);
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", NULL);

    NameContext* ctx = NEW_PTR(NameContext, 1);
    *ctx = { name_type::NONE, name_type::NONE, 0 };

    if (!IN_GLOBAL_NAMESPACE) {
        for (int i = 0; i < context->cur_namespace->size; i++) {
            Name cur_name = context->cur_namespace->names[i];
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

int print_nametable(const NameTable* nametable) {
    ASSERT_IF(VALID_PTR(nametable), "Invalid nametable ptr", -1);

    printf("NameTable-------------------\n");

    print_namespace(&nametable->global);
    for (int i = 0; i < nametable->locals_amount; i++) {
        print_namespace(&nametable->locals[i]);
    }
    printf("----------------------------\n");

    return 1;
}

int print_namespace(const Namespace* namespace_) {
    ASSERT_IF(VALID_PTR(namespace_), "Invalid namespace_ ptr", -1);

    printf("Namespace '%s':\n", namespace_->id);

    for (int i = 0; i < namespace_->size; i++) {
        Name cur_name = namespace_->names[i];

        printf("    Type: %s; Address: [%2d]; Name: '%s'\n", cur_name.type == name_type::VARIABLE ? " VAR" : "FUNC", cur_name.address, cur_name.name);
        if (cur_name.type == name_type::FUNCTION) printf("\t\t\tArgs number: %d\n\n", cur_name.args_amount);
    }

    return 1;
}

int print_std_context(StdContext* std_ctx) {
    ASSERT_IF(VALID_PTR(std_ctx), "Invalid std_ctx ptr", 0);

    printf("Standard context info----------\n");

    printf("\nUsed functions:\n");
    for (int i = 0; i < STD_FUNC_AMOUNT; i++) {
        printf("    %s - %d\n", ALL_STANDARD_FUNCTIONS[i].name, std_ctx->used_functions[i]);
    }
    printf("-------------------------------\n");

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

int is_std_name(CompileContext* context) {
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", 0);

    for (int i = 0; i < STD_FUNC_AMOUNT; i++) {
        if (EQUAL_LEXEM(data_type::VAR_T, ALL_STANDARD_FUNCTIONS[i].name)) {
            return i;
        }
    }

    return -1;
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

    if (EQUAL_LEXEM(data_type::OPR_T, END_STATEMENT)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"), 0);

    if (SOFT_REQUIRE(START_BLOCK)) {
        if (EQUAL_LEXEM(data_type::OPR_T, END_BLOCK)) {
            if (REQUIRE_RETURN) THROW_ERROR("Expected return in function");
            RETURN_WITH_REBIND(END_BLOCK, data_type::OPR_T, strdup(";"), 0);
        }

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
            REBIND_CTX_LEFT(data_type::OPR_T, strdup(";"), 0);
            RETURN_COMPLETED;
        }
        if (REQUIRE_RETURN) THROW_ERROR("Expected return in function");
    }
    
    func_ctx = CALL_RULE(If);
    if (RULE_DONE(func_ctx)) {
        REBIND_CTX_LEFT(data_type::OPR_T, strdup(";"), 0);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(While);
    if (RULE_DONE(func_ctx)) {
        REBIND_CTX_LEFT(data_type::OPR_T, strdup(";"), 0);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(For);
    if (RULE_DONE(func_ctx)) {
        REBIND_CTX_LEFT(data_type::OPR_T, strdup(";"), 0);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Fdef);
    if (RULE_DONE(func_ctx)) {
        REBIND_CTX_LEFT(data_type::OPR_T, strdup(";"), 0);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Vdef);
    if (RULE_DONE(func_ctx)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"), 0);

    func_ctx = CALL_RULE(Exp);
    if (RULE_DONE(func_ctx)) RETURN_WITH_REBIND(END_STATEMENT, data_type::OPR_T, strdup(";"), 0);

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(Fdef) {
    INIT;

    if (!SOFT_REQUIRE("func")) RETURN_NOT_COMPLETED;

    if (IN_FUNCTION) THROW_ERROR("Function definition in another function");

    func_ctx = CALL_RULE(Id);
    if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected func name");
    CHECK_REDEFINITION(MAIN_NAME)

    IN_FUNCTION    = 1;
    REQUIRE_RETURN = 1;
    ADD_NAMESPACE(func_ctx->main_name);
    ADD_NAME(name_type::FUNCTION, MAIN_NAME);

    NAMESPACE = &LOCAL_NAMESPACES[NAME_TABLE.locals_amount - 1];
    ADD_NAME(name_type::FUNCTION, MAIN_NAME);
    REBIND_CTX_LEFT(data_type::VAR_T, strdup("func"), 0);

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

        if (!SOFT_REQUIRE(",")) break;
    }
    HARD_REQUIRE(CLOSE_BRACKET);

    LAST_NAME(GLOBAL_NAMESPACE).args_amount                                              = args_amount;
    LAST_N_NAME(LOCAL_NAMESPACES[NAME_TABLE.locals_amount - 1], args_amount).args_amount = args_amount;

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

    if (!SOFT_REQUIRE("return")) RETURN_NOT_COMPLETED;

    func_ctx = CALL_RULE(Exp);
    if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected Expression after return");

    RETURN_WITH_REBIND(END_STATEMENT, data_type::VAR_T, strdup("return"), 1);
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
    REBIND_CTX_LEFT(data_type::VAR_T, strdup("call"), 1);

    int args_amount = defined_name(MAIN_NAME, context)->args_amount;

    int real_args_am = 0;
    Node* cur_node = func_ctx->node;
    SyntaxContext* args = NULL;
    while (args = CALL_RULE(Exp)) {
        REBIND_NODE(data_type::OPR_T, strdup(";"), 0, args, LEFT);
        add_child(cur_node, args->node, child_type::RIGHT);
        cur_node = args->node;
        real_args_am++;

        if (real_args_am > args_amount) THROW_ERROR("Too many agruments for function");
        if (!SOFT_REQUIRE(",")) break;
    }

   if (real_args_am < args_amount) THROW_ERROR("Not enough arguments for function");

    HARD_REQUIRE(CLOSE_BRACKET);
    RETURN_COMPLETED;
}

GRAMMAR_RULE(StdFunc) {
    INIT;

    int std_name_index = is_std_name(context);
    if (std_name_index == -1) RETURN_NOT_COMPLETED;

    SET_NODE_NAME(data_type::VAR_T, LEXEM_NAME, func_ctx);
    TOKENS_PTR++;

    HARD_REQUIRE(OPEN_BRACKET);
    REBIND_CTX_LEFT(data_type::VAR_T, strdup("call"), 1);

    int       real_args_am = 0;
    Node*         cur_node = func_ctx->node;
    SyntaxContext*    args = NULL;

    StandardFunction* func = NEW_PTR(StandardFunction, 1);
    memcpy(func, &ALL_STANDARD_FUNCTIONS[std_name_index], sizeof(StandardFunction));

    while (args = CALL_RULE(Exp)) {
        REBIND_NODE(data_type::OPR_T, strdup(";"), 0, args, LEFT);
        if (real_args_am == 0) {
            add_child(cur_node, args->node, child_type::RIGHT);
        } else {
            add_child(args->node, cur_node->right, child_type::RIGHT);
            add_child(cur_node, args->node, child_type::RIGHT);
        }
        real_args_am++;

        if (real_args_am > func->max_args) THROW_ERROR("Too many agruments for standard function");
        if (!SOFT_REQUIRE(",")) break;
    }
    if (real_args_am < func->min_args) THROW_ERROR("Not enough arguments for standard function");

    HARD_REQUIRE(CLOSE_BRACKET);

    func->real_args = real_args_am;
    ADD_STD_FUNC(func, std_name_index);
    RETURN_COMPLETED;
}

GRAMMAR_RULE(For) {
    INIT;

    if (SOFT_REQUIRE("for") && SOFT_REQUIRE(OPEN_BRACKET)) {
        SyntaxContext* func_ctx = CALL_RULE(Vdef);
        if (!RULE_DONE(func_ctx)) {
            func_ctx = CALL_RULE(Ass);
            if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected counter definition or assignment in cycle");
            CHECK_UNDEFINITION(MAIN_NAME)
        }
        char* counter_name = MAIN_NAME;

        HARD_REQUIRE("->");
        SyntaxContext* end_ctx = CALL_RULE(Exp);
        if (!RULE_DONE(end_ctx)) THROW_ERROR("Expected expression for end value of counter in cycle");

        Node* step_node = NEW_PTR(Node, 1);
        node_ctor(step_node, NULL, { data_type::CONST_T, 1, 0 });
        if (SOFT_REQUIRE(":")) {
            SyntaxContext* step_exp = CALL_RULE(Exp);
            if (!RULE_DONE(step_exp)) THROW_ERROR("Expected expression for step value");
            step_node = step_exp->node;
        }

        HARD_REQUIRE(CLOSE_BRACKET);
        SyntaxContext* st = CALL_RULE(Statement);
        if (!RULE_DONE(st)) THROW_ERROR("Expected statement in for body");

        /* Creating while tree */
        // Creating continue condition tree
        Node* counter_node = NULL;
        NEW_NAME_NODE(data_type::VAR_T, strdup(counter_name), 0, counter_node);

        REBIND_NODE(data_type::OPR_T, strdup("<"), 1, end_ctx, child_type::RIGHT);
        add_child(end_ctx->node, counter_node, child_type::LEFT);
        //

        // Creating statement tree  Неведомая хрень. Лучше даже не пытаться понять что тут происходит
        SyntaxContext* tmp_ctx = NEW_PTR(SyntaxContext, 1);
        SET_NODE_NAME(data_type::VAR_T, strdup(counter_name), tmp_ctx);

        REBIND_NODE(data_type::OPR_T, strdup("+"), 1, tmp_ctx, child_type::LEFT);
        add_child(tmp_ctx->node, step_node, child_type::RIGHT);
        REBIND_NODE(data_type::OPR_T, strdup("="), 1, tmp_ctx, child_type::RIGHT);

        Node* counter_node1 = NULL;
        NEW_NAME_NODE(data_type::VAR_T, strdup(counter_name), 0, counter_node1);
        add_child(tmp_ctx->node, counter_node1, child_type::LEFT);
        REBIND_NODE(data_type::OPR_T, strdup(";"), 0, tmp_ctx, child_type::RIGHT);
        add_child(st->node, tmp_ctx->node, child_type::RIGHT);
        

        REBIND_NODE(data_type::OPR_T, strdup("if_else"), 0, st, child_type::LEFT);
        REBIND_NODE(data_type::OPR_T, strdup("while"),   1, st, child_type::RIGHT);
        add_child(st->node, end_ctx->node, child_type::LEFT);
        REBIND_NODE(data_type::OPR_T, strdup(";"),       0, st, child_type::LEFT);
        //

        REBIND_CTX_LEFT(data_type::OPR_T, strdup(";"), 0);
        add_child(func_ctx->node, st->node, child_type::RIGHT);
        /* */

        RETURN_COMPLETED;
    }

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(While) {
    INIT;

    if (SOFT_REQUIRE("while")) {
        func_ctx = CALL_RULE(If_cond);
        if (!RULE_DONE(func_ctx)) THROW_ERROR("Expected Statement after while body");

        func_ctx->node->data.value.name = strdup("while");
        RETURN_COMPLETED;
    }

    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(If) {
    INIT;

    if (!SOFT_REQUIRE("if")) RETURN_NOT_COMPLETED;
    
    func_ctx = CALL_RULE(If_cond);
    if (!RULE_DONE(func_ctx)) RETURN_NOT_COMPLETED;

    Node* cur_node = func_ctx->node->right;
    ASSERT_IF(VALID_PTR(cur_node), "Invalid cur_node ptr", NULL);

    while (1) {
        if (!SOFT_REQUIRE("elif")) break;

        SyntaxContext* if_base = CALL_RULE(If_cond);
        if (!RULE_DONE(if_base)) break;
        
        add_child(cur_node, if_base->node, child_type::RIGHT);
        cur_node = if_base->node->right;
        ASSERT_IF(VALID_PTR(cur_node), "Invalid cur_node ptr in cycle", NULL);
    }

    if (SOFT_REQUIRE("else")) {
        SyntaxContext* st = CALL_RULE(Statement);
        if (!RULE_DONE(st)) THROW_ERROR("Expected Statement after else");

        add_child(cur_node, st->node, child_type::RIGHT);
    }

    RETURN_COMPLETED;
}

GRAMMAR_RULE(If_cond) {
    INIT;

    // Try get '(exp)'
    HARD_REQUIRE(OPEN_BRACKET);

    func_ctx = CALL_RULE(Exp);
    if (!RULE_DONE(func_ctx))         THROW_ERROR("Expected expression");

    HARD_REQUIRE(CLOSE_BRACKET);
    //

    // Try get 'Statement'
    SyntaxContext* st = CALL_RULE(Statement);
    if (!RULE_DONE(st)) THROW_ERROR("Expected statement");

    REBIND_CTX_LEFT(data_type::OPR_T, strdup("if"), 1);
    REBIND_NODE(data_type::OPR_T, strdup("if_else"), 0, st, child_type::LEFT);
    add_child(func_ctx->node, st->node, child_type::RIGHT);
    //

    RETURN_COMPLETED;
}

GRAMMAR_RULE(Vdef) {
    INIT;

    if (!SOFT_REQUIRE("def")) RETURN_NOT_COMPLETED;

    func_ctx = CALL_RULE(Ass);
    if (RULE_DONE(func_ctx)) {
        CHECK_REDEFINITION(MAIN_NAME);
        ADD_NAME(name_type::VARIABLE, MAIN_NAME);

        REBIND_CTX_LEFT(data_type::VAR_T, strdup("def"), 0);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Id);
    if (RULE_DONE(func_ctx)) THROW_ERROR("!!!Assignment in variable definition is required (По заветам Деда)!!!");
    
    THROW_ERROR("Expected variable definition");
    RETURN_NOT_COMPLETED;
}

GRAMMAR_RULE(Ass) {
    INIT;

    SyntaxContext* variable   = CALL_RULE(Id);
    if (!RULE_DONE(variable)) RETURN_NOT_COMPLETED;

    if (!SOFT_REQUIRE("=")) {
        TOKENS_PTR--;
        RETURN_NOT_COMPLETED;
    } else if (IS_FUNCTION(variable->main_name)) {
        TOKENS_PTR -= 2;
        THROW_ERROR("Attempt to redefine function as variable");
    }

    SyntaxContext* expression = CALL_RULE(Exp);
    if (!RULE_DONE(expression)) THROW_ERROR("Expected Expression in Assignment");

    SET_NODE_NAME(data_type::OPR_T, strdup("="), func_ctx);
    SET_MAIN_NAME(variable->main_name);

    NODE_SAVING_STATE = 1;
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

    EXPRESSION_PRIORITY_RULE(P, "*", "/", "%");

    RETURN_COMPLETED;
}

GRAMMAR_RULE(P) {
    INIT;

    char* unary_oper = NULL;
    for (int i = 0; i < UNARY_OPERATORS_AMOUNT; i++) {
        if (SOFT_REQUIRE(UNARY_OPERATORS[i])) {
            unary_oper = strdup(UNARY_OPERATORS[i]);
            break;
        }
    }

    if (SOFT_REQUIRE(OPEN_BRACKET)) {
        func_ctx = CALL_RULE(Exp);
        if (!RULE_DONE(func_ctx)) THROW_ERROR("Expexted expression after OPEN_BRACKET");

        if (unary_oper) RETURN_WITH_REBIND(CLOSE_BRACKET, data_type::OPR_T, unary_oper, 1);
        RETURN_WITH_REQUIRE(CLOSE_BRACKET);
    }

    func_ctx = CALL_RULE(Number);
    if (RULE_DONE(func_ctx)) {
        if (unary_oper) REBIND_CTX_LEFT(data_type::OPR_T, unary_oper, 1);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Call);
    if (RULE_DONE(func_ctx)) {
        if (unary_oper) REBIND_CTX_LEFT(data_type::OPR_T, unary_oper, 1);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(StdFunc);
    if (RULE_DONE(func_ctx)) {
        if (unary_oper) REBIND_CTX_LEFT(data_type::OPR_T, unary_oper, 1);
        RETURN_COMPLETED;
    }

    func_ctx = CALL_RULE(Id);
    if (RULE_DONE(func_ctx)) {
        CHECK_UNDEFINITION(MAIN_NAME);
        if (!IS_VARIABLE(MAIN_NAME)) {
            TOKENS_PTR--;
            THROW_ERROR("Not a Variable");
        }

        if (unary_oper) REBIND_CTX_LEFT(data_type::OPR_T, unary_oper, 1);
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

    SET_NODE_NUMBER(data_type::CONST_T, LEXEM_NUM, func_ctx);

    TOKENS_PTR++;
    RETURN_COMPLETED;
}

GRAMMAR_RULE(Id) {
    INIT;

    if (!IS_IDENTIFIER || is_reserved_name(LEXEM_NAME) || is_std_name(context) != -1) {
        RETURN_NOT_COMPLETED;
    }

    SET_NODE_NAME(data_type::VAR_T, LEXEM_NAME, func_ctx);
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

    print_nametable(&NAME_TABLE);

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
