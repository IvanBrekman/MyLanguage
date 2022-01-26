//
// Created by IvanBrekman on 17.01.2022
//

#include <cstring>

#include "asm_gen.h"
#include "asm_genDSL.h"
#include "src/StandardLibrary/functions.h"

int generate_asm_code_from_tree(const FrontContext* front_context) {
    ASSERT_IF(VALID_PTR(front_context), "Invalid context ptr", 0);

    LOG1(printf("Start processing tree...\n"););

    ASMGenerateContext* gen_ctx = NEW_PTR(ASMGenerateContext, 1);
    *gen_ctx = {
        .asm_file       = open_file(ASM_CODE_FILE, "w"),

        .nametable      = front_context->nametable,
        .cur_namespace  = &front_context->nametable->global,
        .std_ctx        = front_context->std_ctx,
        .func_name      = NULL,
        .is_std_func    = 0
    };

    add_standard_library(gen_ctx);

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
    ADD_ASM_CODE("main:\n");
    LOG1(printf("End of standard library.\n\n"););

    return 1;
}

int processing_node(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    if (VALID_PTR(node->left))  processing_node(node->left,  context);
    if (VALID_PTR(node->right)) processing_node(node->right, context);

    LOG2(print_node(node, context););
    if        (IS_NAME)     {
        if        (IS_VARIABLE(NODE_NAME)) {
            ADD_ASM_CODE("# add variable '%s' data to stack", NODE_NAME);
            ADD_ASM_CODE("push [%d]\n", get_variable_address(node, context));
        } else if (IS_FUNCTION(NODE_NAME) || IS_STD_FUNCTION) {
            context->func_name   = NODE_NAME;
            context->is_std_func = IS_STD_FUNCTION;
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

    return 1;
}

int processing_operator(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;
    ASSERT_IF(IS_OPERATOR, "Cant process non operator node", 0);

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

    else if (EQUAL_OPER("=")) {
        ADD_ASM_CODE("pop [%d]", get_variable_address(node->left, context));
        ADD_ASM_CODE("pop");
    }

    else if (EQUAL_OPER(";")) return 1;
    else ASSERT_IF(0, "Unknown operator", 0);

    ADD_ASM_CODE(" ");

    return 1;
}

int processing_special_name(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    if        (EQUAL(NODE_NAME, "call")) {
        ADD_ASM_CODE("# add 'call' command for %sfunc", context->is_std_func ? "standard " : "");
        ADD_ASM_CODE("call %s", node->left->data.value.name);

        if (context->is_std_func) {
            for (int i = 1; i < CURRENT_STD_FUNC.real_args; i++) {
                ADD_ASM_CODE("pop  # delete print result from stack");
                ADD_ASM_CODE("call %s", node->left->data.value.name);
            }
            ADD_ASM_CODE("# end of 'call' command for std function\n");
            STD_FUNC_PTR++;
        } else ADD_ASM_CODE(" ");
    } else if (EQUAL(NODE_NAME, "return")) {
        ADD_ASM_CODE("ret\n");
    } else {
        LOG1(printf(RED "Name: '%s'\n" NATURAL, NODE_NAME););
        ASSERT_IF(0, "Unknown special name", 0);
    }

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

int check_name_type(const Node* node, ASMGenerateContext* context, const char* name, name_type type) {
    ASSERT_CONTEXT;
    ASSERT_IF(VALID_PTR(name), "Invalid name ptr", 0);

    if (!IN_GLOBAL_NAMESPACE) {
        for (int i = 0; i < CURRENT_NAMESPACE->size; i++) {
            if (EQUAL(CURRENT_NAMESPACE->names[i].name, NODE_NAME)) {
                return CURRENT_NAMESPACE->names[i].type == type;
            }
        }
    }

    for (int i = 0; i < GLOBAL_NAMESPACE.size; i++) {
        if (EQUAL(GLOBAL_NAMESPACE.names[i].name, NODE_NAME)) {
            return GLOBAL_NAMESPACE.names[i].type == type;
        }
    }

    return 0;
}

int is_std_func(const Node* node, ASMGenerateContext* context) {
    ASSERT_CONTEXT;

    for (int i = 0; i < STD_FUNC_AMOUNT; i++) {
        if (IS_NAME && EQUAL(NODE_NAME, ALL_STANDARD_FUNCTIONS[i].name)) {
            return i;
        }
    }

    return -1;
}
