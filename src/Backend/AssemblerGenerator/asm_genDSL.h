//
// Created by IvanBrekman on 17.01.2022
//

#define DATA                node->data
#define NODE_TYPE           DATA.type
#define NODE_NAME           DATA.value.name
#define NODE_NUMBER         DATA.value.number

#define LEFT_NODE_DATA      node->left->data
#define LEFT_NODE_TYPE      LEFT_NODE_DATA.type
#define LEFT_NODE_NAME      LEFT_NODE_DATA.value.name

#define IS_NUMBER           (NODE_TYPE == data_type::CONST_T)
#define IS_OPERATOR         (NODE_TYPE == data_type::OPR_T)
#define IS_NAME             (NODE_TYPE == data_type::VAR_T)

#define NAMETABLE           context->nametable
#define GLOBAL_NAMESPACE    NAMETABLE->global
#define CURRENT_NAMESPACE   context->cur_namespace

#define ASM_INDENT          context->asm_indent
#define STD_CONTEXT         context->std_ctx
#define STD_FUNC_PTR        STD_CONTEXT->func_ptr
#define CURRENT_STD_FUNC    STD_CONTEXT->all_functions[STD_FUNC_PTR]

#define IN_GLOBAL_NAMESPACE (CURRENT_NAMESPACE == &GLOBAL_NAMESPACE)
#define FAN_VAN             printf("Press F to Fan Van")

#define ADD_ASM_CODE(format...) {                               \
    if (ASM_INDENT > 0) {                                       \
        SPR_FPUTS(context->asm_file, "%*s", ASM_INDENT, " ");   \
    }                                                           \
    SPR_FPUTS(context->asm_file, format);                       \
    SPR_FPUTS(context->asm_file, "\n");                         \
}

#define ASSERT_CONTEXT {                                        \
    ASSERT_CTX;                                                 \
    ASSERT_IF(VALID_PTR(node), "Invalid node ptr", 0);          \
}

#define ASSERT_CTX {                                                                \
    ASSERT_IF(VALID_PTR(context),                "Invalid context ptr",       0);   \
    ASSERT_IF(VALID_PTR(context->asm_file),      "Invalid asm_file ptr",      0);   \
    ASSERT_IF(VALID_PTR(context->nametable),     "Invalid nametable ptr",     0);   \
    ASSERT_IF(VALID_PTR(context->cur_namespace), "Invalid cur_namespace ptr", 0);   \
    ASSERT_IF(VALID_PTR(context->std_ctx),       "Invalid std_ctx ptr",       0);   \
}

#define EQUAL(string1, string2) (strcmp(string1, string2) == 0)
#define EQUAL_OPER(operator)    EQUAL(NODE_NAME, operator)
#define UNARY_OPER              (!(VALID_PTR(node->left) && VALID_PTR(node->right)))

#define IS_VARIABLE(name)   check_name_type(node, context, name, name_type::VARIABLE)
#define IS_FUNCTION(name)   check_name_type(node, context, name, name_type::FUNCTION)
#define IS_STD_FUNCTION     is_std_func(node, context)

#define PROCESS_COMPARISON(jmp, index) {        \
    ADD_ASM_CODE("%s add1_%d", jmp, index);     \
    ADD_ASM_CODE("push 0");                     \
    ADD_ASM_CODE("jmp next_%d\n", index);       \
                                                \
    ADD_ASM_CODE("add1_%d:", index);            \
    ADD_ASM_CODE("   push 1");                  \
    ADD_ASM_CODE("next_%d:\n", index);          \
}
