//
// Created by IvanBrekman on 17.01.2022
//

#include <cstring>

#include "asm_gen.h"
#include "asm_genDSL.h"
#include "src/StandardLibrary/functions.h"

int node_counter = 0;

int generate_asm_code_from_tree(const FrontContext* front_context) {
    ASSERT_IF(VALID_PTR(front_context), "Invalid context ptr", 0);

    LOG1(printf("Start processing tree...\n"););

    ASMGenerateContext* gen_ctx = NEW_PTR(ASMGenerateContext, 1);
    *gen_ctx = {
        .asm_file       = open_file(ASM_CODE_FILE, "w"),

        .nametable      = front_context->nametable,
        .cur_namespace  = &front_context->nametable->global,
        .std_ctx        = front_context->std_ctx,
    };

    add_standard_library(gen_ctx);
    add_user_functions(front_context->AST_tree->root, gen_ctx);

    SPR_FPUTS(gen_ctx->asm_file, "main:\n");
    processing_node(front_context->AST_tree->root, gen_ctx);
    SPR_FPUTS(gen_ctx->asm_file, "\nhlt\n");

    close_file(gen_ctx->asm_file);
    LOG1(printf("Successful processing\n"););

    return 1;
}

int add_standard_library(const ASMGenerateContext* context) {
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", 0);

    LOG1(printf("\nAdding standard library.\n"););
    ADD_ASM_CODE("jmp main\n");
    ADD_ASM_CODE("# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #");
    ADD_ASM_CODE("#                             STANDARD LIBRARY FUNCTIONS\n");

    for (int i = 0; i < STD_FUNC_AMOUNT; i++) {
        LOGN(3, printf("    Function '%s' (%s)\n", ALL_STANDARD_FUNCTIONS[i].name,
                       STD_CONTEXT->used_functions[i] ? (GREEN "Added" NATURAL) : (RED "Skipped" NATURAL)
        ););

        if (STD_CONTEXT->used_functions[i]) {
            ALL_STANDARD_FUNCTIONS[i].execute_func(context->asm_file);
        }
    }

    ADD_ASM_CODE("# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #\n");
    LOG1(printf("End of standard library.\n\n"););

    return 1;
}

int add_user_functions(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    if (VALID_PTR(node->left))  add_user_functions(node->left,  context);
    if (VALID_PTR(node->right)) add_user_functions(node->right, context);

    if (IS_NAME && EQUAL(NODE_NAME, "func")) processing_func_node(node, context);

    return 1;
}

int processing_node(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    if (IS_NAME     && EQUAL(NODE_NAME, "func")) return 1;
    if (IS_OPERATOR && EQUAL(NODE_NAME, "if")) {
        processing_if_node(node, context);
        node_counter++;
        return 1;
    }

    if (VALID_PTR(node->left))  processing_node(node->left,  context);
    if (VALID_PTR(node->right)) processing_node(node->right, context);

    LOG2(print_node(node, context); printf("\n"););
    if        (IS_NAME)     {
        if        (IS_VARIABLE(NODE_NAME)) {
            ADD_ASM_CODE("# add variable '%s' data to stack", NODE_NAME);
            ADD_ASM_CODE("push [%d]\n", get_variable_address(node, context));
        } else if (IS_FUNCTION(NODE_NAME) || IS_STD_FUNCTION) {
            // nothing //
        } else {
            processing_special_name(node, context);
        }
    } else if (IS_OPERATOR) {
        processing_operator(node, context);
    } else if (IS_NUMBER)   {
        ADD_ASM_CODE("# add number to stack");
        ADD_ASM_CODE("push %d\n", NODE_NUMBER);
    } else {
        ASSERT_IF(0, "Invalid node type", 0);
    }

    node_counter++;
    return 1;
}

int processing_operator(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;
    ASSERT_IF(IS_OPERATOR, "Cant process non operator node", 0);

    if (EQUAL_OPER(";")) return 1;

    ADD_ASM_CODE("# processing '%s' operator", NODE_NAME);

    if      (EQUAL_OPER("+")) { ADD_ASM_CODE("add"); }
    else if (EQUAL_OPER("-")) {
        if (UNARY_OPER) {
            ADD_ASM_CODE("# unary minus");
            ADD_ASM_CODE("push -1");
            ADD_ASM_CODE("mul");
        } else {
            ADD_ASM_CODE("sub");
        }
    }
    else if (EQUAL_OPER("*")) { ADD_ASM_CODE("mul"); }
    else if (EQUAL_OPER("/")) { ADD_ASM_CODE("div"); }

    else if (EQUAL_OPER(">"))  { PROCESS_COMPARISON("jg",  node_counter); }
    else if (EQUAL_OPER("<"))  { PROCESS_COMPARISON("jl",  node_counter); }
    else if (EQUAL_OPER("==")) { PROCESS_COMPARISON("je",  node_counter); }
    else if (EQUAL_OPER("!=")) { PROCESS_COMPARISON("jne", node_counter); }
    else if (EQUAL_OPER(">=")) { PROCESS_COMPARISON("jge", node_counter); }
    else if (EQUAL_OPER("<=")) { PROCESS_COMPARISON("jle", node_counter); }

    else if (EQUAL_OPER("=")) {
        int address = get_variable_address(node->left, context);
        ADD_ASM_CODE("pop [%d]", address);
        ADD_ASM_CODE("pop");
        ADD_ASM_CODE("push [%d]", address);
    }

    else ASSERT_IF(0, "Unknown operator", 0);

    ADD_ASM_CODE(" ");
    return 1;
}

int processing_special_name(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    if        (EQUAL(NODE_NAME, "call")) {
        int std_func_flag = is_std_func(node->left, context);
        int args_am = 0;

        const Node* cur_node = node;
        while (VALID_PTR(cur_node->right)) {
            cur_node = cur_node->right;
            args_am++;
        }

        ADD_ASM_CODE("# add 'call' command for %sfunc", std_func_flag ? "standard " : "");
        ADD_ASM_CODE("call %s", LEFT_NODE_NAME);

        if (std_func_flag) {
            for (int i = 1; i < args_am; i++) {
                ADD_ASM_CODE("pop  # delete print result from stack");
                ADD_ASM_CODE("call %s", LEFT_NODE_NAME);
            }
            ADD_ASM_CODE("# end of 'call' command for std function (%s)\n", LEFT_NODE_NAME);
        } else ADD_ASM_CODE(" ");

        cur_node = node->parent;
        while (VALID_PTR(cur_node->parent) && EQUAL(cur_node->parent->data.value.name, ";")) cur_node = cur_node->parent;
        if (!VALID_PTR(cur_node->parent)) {
            ADD_ASM_CODE("# Unused function result. Delete result from stack.")
            ADD_ASM_CODE("pop\n");
        }
    }
    else if (EQUAL(NODE_NAME, "return")) {
        ADD_ASM_CODE("ret\n");
    }
    else if (EQUAL(NODE_NAME, "def")) {
        ADD_ASM_CODE("# clear stack after variable definition")
        ADD_ASM_CODE("pop\n");
    }
    else {
        LOG1(printf(RED "Name: '%s'\n" NATURAL, NODE_NAME););
        ASSERT_IF(0, "Unknown special name", 0);
    }

    return 1;
}

int processing_func_node(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;
    ASSERT_IF(LEFT_NODE_TYPE == data_type::VAR_T, "Incorrect left node type", 0);

    // Поиск и смена текущего Namespace
    char* func_name = LEFT_NODE_NAME;
    ADD_ASM_CODE("# user func '%s'", func_name);
    ADD_ASM_CODE("%s:", func_name);

    for (int i = 0; i < NAMETABLE->locals_amount; i++) {
        if (EQUAL(func_name, NAMETABLE->locals[i].id)) {
            CURRENT_NAMESPACE = &NAMETABLE->locals[i];
        }
    }
    //

    // Получение объекта функции, количества аргументов и массива имен аргументов
    Name* name      = get_name_obj(node, context, func_name);
    ASSERT_IF(VALID_PTR(name), "Invalid name ptr. Don't find name.", 0);

    Node** args_names = NEW_PTR(Node*, name->args_amount);
    int    args_am    = name->args_amount;
    Node* cur_node = node->left;

    while (VALID_PTR(cur_node->left)) {
        args_names[--args_am] = cur_node->left;
        cur_node = cur_node->left;
    }
    //

    ADD_ASM_CODE("# load params to RAM");
    for (int i = 0; i < name->args_amount; i++) {
        ADD_ASM_CODE("pop [%d]", get_variable_address(args_names[i], context));
    }
    ADD_ASM_CODE("# ------------------\n");

    ADD_ASM_CODE("# ++++++++++processing func logic++++++++++");
    processing_node(node->right, context);

    CURRENT_NAMESPACE = &GLOBAL_NAMESPACE;
    ADD_ASM_CODE("# ++++++++++end of func logic++++++++++\n");

    return 1;
}

int processing_if_node(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    int index = node_counter;

    ADD_ASM_CODE("# if_%d processing _________________\n", index);
    ADD_ASM_CODE("# if condition----------");
    processing_node(node->left, context);
    ADD_ASM_CODE("# ----------------------\n");

    ADD_ASM_CODE("push 0");
    ADD_ASM_CODE("je else_%d\n", index);

    ADD_ASM_CODE("# if_%d:", index);
    processing_node(node->right->left, context);
    ADD_ASM_CODE("jmp next_%d\n", index);
 
    ADD_ASM_CODE("else_%d:", index);
    if (VALID_PTR(node->right->right)) processing_node(node->right->right, context);
    ADD_ASM_CODE("next_%d:", index);

    ADD_ASM_CODE("# endif_%d _________________________", index);

    return 1;
}

int get_variable_address(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;
    ASSERT_IF(IS_NAME, "Cant get address of non name type of node", 0);

    if (!IN_GLOBAL_NAMESPACE) {
        for (int i = 0; i < CURRENT_NAMESPACE->size; i++) {
            if (EQUAL(CURRENT_NAMESPACE->names[i].name, NODE_NAME)) {
                LOGN(3, printf("Find variable with name '%s' (local). Address: %d\n", NODE_NAME, CURRENT_NAMESPACE->names[i].address););
                return CURRENT_NAMESPACE->names[i].address;
            }
        }
    }

    for (int i = 0; i < GLOBAL_NAMESPACE.size; i++) {
        if (EQUAL(GLOBAL_NAMESPACE.names[i].name, NODE_NAME)) {
            LOGN(3, printf("Find variable with name '%s' (global). Address: %d\n", NODE_NAME, CURRENT_NAMESPACE->names[i].address););
            return GLOBAL_NAMESPACE.names[i].address;
        }
    }

    ASSERT_IF(0, "Cant find this name in any namespace", 0);
    return 0;
}

int print_node(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    if (IS_NAME || IS_OPERATOR) {
        printf("Node. Type: %s; value: '%s'\n", IS_NAME ? "NAME" : "OPER", NODE_NAME);
    } else {
        printf("Node. Type: %s; value:  %d\n", !IS_NUMBER ? (RED "ERROR" NATURAL) : " NUM", NODE_NUMBER);
        ASSERT_IF(IS_NUMBER, "Bad node type", 0);
    }
    
    return 1;
}

Name* get_name_obj(const Node* node, ASMGenerateContext* context, const char* name) {
    ASSERT_CONTEXT;
    ASSERT_IF(VALID_PTR(name), "Invalid name ptr", 0);

    if (!IN_GLOBAL_NAMESPACE) {
        for (int i = 0; i < CURRENT_NAMESPACE->size; i++) {
            if (EQUAL(CURRENT_NAMESPACE->names[i].name, name)) {
                return &CURRENT_NAMESPACE->names[i];
            }
        }
    }

    for (int i = 0; i < GLOBAL_NAMESPACE.size; i++) {
        if (EQUAL(GLOBAL_NAMESPACE.names[i].name, name)) {
            return &GLOBAL_NAMESPACE.names[i];
        }
    }

    return NULL;
}

int check_name_type(const Node* node, ASMGenerateContext* context, const char* name, name_type type) {
    ASSERT_CONTEXT;
    ASSERT_IF(VALID_PTR(name), "Invalid name ptr", 0);

    Name* obj = get_name_obj(node, context, name);
    if (VALID_PTR(obj)) return obj->type == type;

    return 0;
}

int is_std_func(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    for (int i = 0; i < STD_FUNC_AMOUNT; i++) {
        if (IS_NAME && EQUAL(NODE_NAME, ALL_STANDARD_FUNCTIONS[i].name)) {
            return 1;
        }
    }

    return 0;
}
