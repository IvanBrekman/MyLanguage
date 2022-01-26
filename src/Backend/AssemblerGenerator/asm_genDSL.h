//
// Created by IvanBrekman on 17.01.2022
//

#define DATA                node->data
#define NODE_TYPE           DATA.type
#define NODE_NAME           DATA.value.name
#define NODE_NUMBER         DATA.value.number

#define IS_NUMBER           (NODE_TYPE == data_type::CONST_T)
#define IS_OPERATOR         (NODE_TYPE == data_type::OPR_T)
#define IS_NAME             (NODE_TYPE == data_type::VAR_T)

#define NAMETABLE           context->nametable
#define GLOBAL_NAMESPACE    NAMETABLE->global
#define CURRENT_NAMESPACE   context->cur_namespace

#define STD_CONTEXT         context->std_ctx
#define STD_FUNC_PTR        STD_CONTEXT->func_ptr
#define CURRENT_STD_FUNC    STD_CONTEXT->all_functions[STD_FUNC_PTR]

#define IN_GLOBAL_NAMESPACE (CURRENT_NAMESPACE == &GLOBAL_NAMESPACE)

#define ADD_ASM_CODE(format...) {                   \
    if (!IN_GLOBAL_NAMESPACE) {                     \
        SPR_FPUTS(context->asm_file, "    ");       \
    }                                               \
    SPR_FPUTS(context->asm_file, format);           \
    SPR_FPUTS(context->asm_file, "\n");             \
}

#define ASSERT_CONTEXT {                                                            \
    ASSERT_CTX;                                         \
    ASSERT_IF(VALID_PTR(node), "Invalid node ptr", 0);  \
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
#define IS_STD_FUNCTION     (is_std_func(node, context) != -1)
