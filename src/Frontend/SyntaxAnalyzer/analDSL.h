//
// Created by IvanBrekman on 18.12.2021
//

#define TOKENS      context.tokens
#define TOKENS_PTR  context.pointer
#define TOKENS_SIZE TOKENS->size

#define LEXEM       TOKENS->data[TOKENS_PTR]
#define LEXEM_TYPE  LEXEM.type
#define LEXEM_NAME  LEXEM.data.name
#define LEXEM_NUM   LEXEM.data.number

#define EQUAL(string1, string2)     (strcmp(string1, string2) == 0)
#define EQUAL_LEXEM(_type, string)  (LEXEM_TYPE == _type && EQUAL(LEXEM_NAME, string))

#define IS_NUMBER     (LEXEM_TYPE == data_type::CONST_T)
#define IS_IDENTIFIER (LEXEM_TYPE == data_type::VAR_T)
#define IS_OPERATOR   (LEXEM_TYPE == data_type::OPR_T)

#define IS_VARIABLE(name)  (defined_name(name) == name_type::VARIABLE)
#define IS_FUNCTION(name)  (defined_name(name) == name_type::FUNCTION)
#define IS_UNDEFINED(name) (defined_name(name) == name_type::NONE)

#define ADD_NAME(_type)     name_table.names[name_table.size++] = { MAIN_NAME, _type }

#define MAIN_NAME             func_ctx->main_name
#define SET_MAIN_NAME(_value) MAIN_NAME = _value;

#define SET_NODE(_type, _value, cnt, _union_type) { \
    cnt->node = NEW_PTR(Node);                      \
    node_ctor(cnt->node, NULL, { _type, 0 });       \
    cnt->node->data.value._union_type  = _value;    \
}

#define SET_NODE_NAME(_type, _value)    SET_NODE(_type, _value, func_ctx, name);
#define SET_NODE_NUMBER(_type, _value ) SET_NODE(_type, _value, func_ctx, number);

#define GRAMMAR_RULE(name)      SyntaxContext* Rule_ ## name()
#define    CALL_RULE(name)      Rule_ ## name()
#define    RULE_DONE(cnt)       (cnt != NULL)

#define REBIND_NODE(_type, _value, cnt, _child_type) {  \
    Node* tmp = cnt->node;                              \
    SET_NODE(_type, _value, cnt, name);                 \
    add_child(cnt->node, tmp, _child_type);             \
}

#define REBIND_CNT_LEFT(_type, _value) REBIND_NODE(_type, _value, func_ctx, child_type::LEFT)
#define CTX_TOKEN_SHIFT                func_ctx->token_shift

#define INIT                                                    \
    LOGN(3, printf(PURPLE "Start %-40s -> " NATURAL "%s:%d\n",  \
                   __FUNCTION__, __FILE__, __LINE__););         \
    SyntaxContext* func_ctx = NEW_PTR(SyntaxContext);           \
    int start_ptr = TOKENS_PTR

#define RETURN_COMPLETED {                                              \
    LOGN(3, printf(PURPLE "  End %-35s DONE -> " NATURAL "%s:%d\n",     \
                   __FUNCTION__, __FILE__, __LINE__););                 \
    CTX_TOKEN_SHIFT = TOKENS_PTR - start_ptr;                           \
    return func_ctx;                                                    \
}

#define RETURN_NOT_COMPLETED {                                          \
    LOGN(3, printf(PURPLE "  End %-31s NOT DONE -> " NATURAL "%s:%d\n", \
                   __FUNCTION__, __FILE__, __LINE__););                 \
    return NULL;                                                        \
}

#define RETURN_WITH_REQUIRE(symbol) { HARD_REQUIRE(symbol); RETURN_COMPLETED; }

#define RETURN_WITH_REBIND(symbol, _type, _value) {     \
    HARD_REQUIRE(symbol);                               \
    REBIND_CNT_LEFT(_type, _value);                     \
    RETURN_COMPLETED;                                   \
}

#define HARD_REQUIRE(symbol) Require(symbol, REQUIRED,       func_ctx)
#define SOFT_REQUIRE(symbol) Require(symbol, NON_REQUIRED,   func_ctx)

#define CHECK_REDEFINITION(var_type) if (!IS_UNDEFINED(MAIN_NAME)) { TOKENS_PTR -= CTX_TOKEN_SHIFT; SyntaxError("Redefinition of " var_type); }
#define CHECK_UNDEFINITION(var_type) if ( IS_UNDEFINED(MAIN_NAME)) { TOKENS_PTR -= CTX_TOKEN_SHIFT; SyntaxError("Undefined "       var_type); }
