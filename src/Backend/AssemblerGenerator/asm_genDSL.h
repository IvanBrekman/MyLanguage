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

#define ASM_FILE            context->asm_file
#define ASM_INDENT          context->asm_indent
#define ASM_LENGTH          context->asm_length
#define STD_CONTEXT         context->std_ctx
#define STD_FUNC_PTR        STD_CONTEXT->func_ptr
#define CURRENT_STD_FUNC    STD_CONTEXT->all_functions[STD_FUNC_PTR]

#define IN_GLOBAL_NAMESPACE (CURRENT_NAMESPACE == &GLOBAL_NAMESPACE)
#define FAN_VAN             printf("Press F to Fan Van")

#define GENERATE_SPR_STRING(file, format...) {              \
    char* string = NEW_PTR(char, MAX_SPRINTF_STRING_SIZE);  \
    sprintf(string, format);                                \
                                                            \
    fputs(string, file);                                    \
                                                            \
    ASM_LENGTH += strlen(string);                           \
    FREE_PTR(string, char);                                 \
}

#define ADD_ASM_CODE(format...) {           \
    FILL_INDENT;                            \
    GENERATE_SPR_STRING(ASM_FILE, format);  \
}

#define PART_LINE_COMMENT(format...) {                                          \
    GENERATE_SPR_STRING(ASM_FILE, "%*s", ASM_COMMENTS_START - ASM_LENGTH, " "); \
    GENERATE_SPR_STRING(ASM_FILE, "%c ", ASM_COMMENT_SYMBOL);                   \
    GENERATE_SPR_STRING(ASM_FILE, format);                                      \
    GENERATE_SPR_STRING(ASM_FILE, "\n");                                        \
    ASM_LENGTH = 0;                                                             \
}

#define FULL_LINE_COMMENT(format...) {                          \
    FILL_INDENT;                                                \
                                                                \
    GENERATE_SPR_STRING(ASM_FILE, "%c ", ASM_COMMENT_SYMBOL);   \
    GENERATE_SPR_STRING(ASM_FILE, format);                      \
    GENERATE_SPR_STRING(ASM_FILE, "\n")                         \
    ASM_LENGTH = 0;                                             \
}

#define FILL_INDENT {                                           \
    if (ASM_LENGTH > 0) {                                       \
        GENERATE_SPR_STRING(ASM_FILE, "\n");                    \
        ASM_LENGTH = 0;                                         \
    }                                                           \
    if (ASM_INDENT > 0) {                                       \
        GENERATE_SPR_STRING(ASM_FILE, "%*s", ASM_INDENT, " ");  \
    }                                                           \
}

#define ASSERT_CONTEXT {                                        \
    ASSERT_CTX;                                                 \
    ASSERT_IF(VALID_PTR(node), "Invalid node ptr", 0);          \
}

#define ASSERT_CTX {                                                            \
    ASSERT_IF(VALID_PTR(context),           "Invalid context ptr",       0);    \
    ASSERT_IF(VALID_PTR(ASM_FILE),          "Invalid asm_file ptr",      0);    \
    ASSERT_IF(VALID_PTR(NAMETABLE),         "Invalid nametable ptr",     0);    \
    ASSERT_IF(VALID_PTR(CURRENT_NAMESPACE), "Invalid cur_namespace ptr", 0);    \
    ASSERT_IF(VALID_PTR(STD_CONTEXT),       "Invalid std_ctx ptr",       0);    \
}

#define EQUAL(string1, string2) (strcmp(string1, string2) == 0)
#define EQUAL_OPER(operator)    EQUAL(NODE_NAME, operator)
#define UNARY_OPER              (!(VALID_PTR(node->left) && VALID_PTR(node->right)))

#define IS_VARIABLE(name)   check_name_type(node, context, name, name_type::VARIABLE)
#define IS_FUNCTION(name)   check_name_type(node, context, name, name_type::FUNCTION)
#define IS_STD_FUNCTION     is_std_func(node, context)

#define PROCESS_COMPARISON(jmp, index) {            \
    ADD_ASM_CODE("%s add1_%d", jmp, index);         \
    PART_LINE_COMMENT("Processing comparison\n");   \
                                                    \
    ADD_ASM_CODE("push 0");                         \
    PART_LINE_COMMENT("False result");              \
    ADD_ASM_CODE("jmp next_%d\n", index);           \
                                                    \
    ADD_ASM_CODE("add1_%d:", index);                \
    ADD_ASM_CODE("    push 1");                     \
    PART_LINE_COMMENT("True result");               \
    ADD_ASM_CODE("next_%d:", index);                \
    FULL_LINE_COMMENT("~~~~~~~~~~~~~~~~~~~~~~~~");  \
}
