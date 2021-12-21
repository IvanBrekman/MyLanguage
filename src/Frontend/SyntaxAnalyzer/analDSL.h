//
// Created by IvanBrekman on 18.12.2021
//

#define GRAMMAR_RULE(name)      SyntaxContext* Rule_ ## name(CompileContext* context)
#define    CALL_RULE(name)      Rule_ ## name(context)
#define    RULE_DONE(cnt)       (cnt != NULL)
#define THROW_ERROR(text)       SyntaxError(text, context);

#define NAME_TABLE          context->name_table
#define GLOBAL_NAMESPACE    NAME_TABLE.global
#define LOCAL_NAMESPACES    NAME_TABLE.locals
#define NAMESPACE           context->cur_namespace
#define FILEPATH            context->src_filepath
#define IN_FUNCTION         context->in_function
#define REQUIRE_RETURN      context->require_return
#define LAST_NAME(nmsp)     nmsp.names[nmsp.size - 1]

#define ADD_NAMESPACE(name) LOCAL_NAMESPACES[NAME_TABLE.locals_amount++] = { name, NEW_PTR(Name, MAX_NAMES_AMOUNT), 0 }
#define IN_GLOBAL_NAMESPACE (EQUAL(NAMESPACE->id, GLOBAL_NAMESPACE.id))

#define TOKENS      context->prs_ctx.tokens
#define TOKENS_PTR  context->prs_ctx.pointer
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

#define ADD_NAME(_type, name)     NAMESPACE->names[NAMESPACE->size++] = { name, _type }

#define MAIN_NAME             func_ctx->main_name
#define SET_MAIN_NAME(_value) MAIN_NAME = _value;

#define SET_NODE(_type, _value, cnt, _union_type) { \
    cnt->node = NEW_PTR(Node, 1);                   \
    node_ctor(cnt->node, NULL, { _type, 0 });       \
    cnt->node->data.value._union_type  = _value;    \
}

#define SET_NODE_NAME(_type, _value)    SET_NODE(_type, _value, func_ctx, name);
#define SET_NODE_NUMBER(_type, _value ) SET_NODE(_type, _value, func_ctx, number);

#define REBIND_NODE(_type, _value, cnt, _child_type) {  \
    Node* tmp = cnt->node;                              \
    SET_NODE(_type, _value, cnt, name);                 \
    add_child(cnt->node, tmp, _child_type);             \
}

#define REBIND_CNT_LEFT(_type, _value) REBIND_NODE(_type, _value, func_ctx, child_type::LEFT)
#define CTX_TOKEN_SHIFT                func_ctx->token_shift

#define INIT                                                    \
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", NULL); \
    LOGN(3, printf(PURPLE "Start %-40s -> " NATURAL "%s:%d\n",  \
                   __FUNCTION__, __FILE__, __LINE__););         \
    SyntaxContext* func_ctx = NEW_PTR(SyntaxContext, 1);        \
    int start_ptr = TOKENS_PTR

#define RETURN_COMPLETED {                                              \
    LOGN(3, printf(PURPLE "  End %-35s DONE -> " NATURAL "%s:%d\n",     \
                   __FUNCTION__, __FILE__, __LINE__););                 \
    CTX_TOKEN_SHIFT = TOKENS_PTR - start_ptr;                           \
    return func_ctx;                                                    \
}

#define RETURN_NOT_COMPLETED {                                          \
    LOGN(3, printf(PURPLE "  End %-31s NOT DONE -> " NATURAL "%s:%d %d\n", \
                   __FUNCTION__, __FILE__, __LINE__, TOKENS_PTR););                 \
    return NULL;                                                        \
}

#define RETURN_WITH_REQUIRE(symbol) { HARD_REQUIRE(symbol); RETURN_COMPLETED; }

#define RETURN_WITH_REBIND(symbol, _type, _value) {     \
    HARD_REQUIRE(symbol);                               \
    REBIND_CNT_LEFT(_type, _value);                     \
    RETURN_COMPLETED;                                   \
}

#define HARD_REQUIRE(symbol) (context->require_state =     REQUIRED, Require(symbol, context))
#define SOFT_REQUIRE(symbol) (context->require_state = NON_REQUIRED, Require(symbol, context))

#define IS_VARIABLE(name)  check_type(name, context, name_type::VARIABLE)
#define IS_FUNCTION(name)  check_type(name, context, name_type::FUNCTION)
#define IS_UNDEFINED(name) (!defined_name(name, context)->is_defined)

#define CHECK_REDEFINITION(name) if (check_redefinition(name, context)) { TOKENS_PTR -= CTX_TOKEN_SHIFT; THROW_ERROR("Redefinition of name"); }
#define CHECK_UNDEFINITION(name) if (IS_UNDEFINED(name))                { TOKENS_PTR -= CTX_TOKEN_SHIFT; THROW_ERROR("Undefined name"      ); }

