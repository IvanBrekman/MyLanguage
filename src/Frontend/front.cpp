//
// Created by IvanBrekman on 13.12.2021
//

#include "config.h"
#include "libs/baselib.h"

#include "front.h"
#include "Tokenizer/token.h"
#include "SyntaxAnalyzer/anal.h"

int Front(const char* filepath) {
    ASSERT_IF(filepath, "invalid filepath ptr", 0);

    Tokens* tokens = get_tokens(filepath);
    tokens_dump(tokens, "Check tokens");
    WAIT_INPUT;

#if 0
    Tree tree = { };
    tree_ctor(&tree);

    Node node1 = { };
    node_ctor(&node1, NULL, { data_type::OPR_T, 0 });
    node1.data.value.name = "if";

    Node node2 = { };
    node_ctor(&node2, NULL, { data_type::OPR_T, 0 });
    node2.data.value.name = "==";

    Node node3 = { };
    node_ctor(&node3, NULL, { data_type::VAR_T, 0 });
    node3.data.value.name = "a";

    Node node4 = { };
    node_ctor(&node4, NULL, { data_type::CONST_T, 0 });
    node4.data.value.number = 2;

    Node node5 = { };
    node_ctor(&node5, NULL, { data_type::OPR_T, 0 });
    node5.data.value.name = "if_else";

    Node node6 = { };
    node_ctor(&node6, NULL, { data_type::OPR_T, 0 });
    node6.data.value.name = ";";

    Node node7 = { };
    node_ctor(&node7, NULL, { data_type::OPR_T, 0 });
    node7.data.value.name = "=";

    Node node8 = { };
    node_ctor(&node8, NULL, { data_type::VAR_T, 0 });
    node8.data.value.name = "n";

    Node node9 = { };
    node_ctor(&node9, NULL, { data_type::CONST_T, 0 });
    node9.data.value.number = 3;
    
    set_new_root(&tree, &node1);

    ADD_CHILD(tree, node1, node2, child_type::LEFT);
    ADD_CHILD(tree, node1, node5, child_type::RIGHT);
    ADD_CHILD(tree, node2, node3, child_type::LEFT);
    ADD_CHILD(tree, node2, node4, child_type::RIGHT);
    ADD_CHILD(tree, node5, node6, child_type::LEFT);
    ADD_CHILD(tree, node6, node7, child_type::LEFT);
    ADD_CHILD(tree, node7, node8, child_type::LEFT);
    ADD_CHILD(tree, node7, node9, child_type::RIGHT);

    Tree_dump(&tree, "check tree");
    LOG_DUMP_GRAPH(&tree, "Check tree", Tree_dump_graph);
    getchar();
#endif

    Tree* AST_tree = build_ast_tree(tokens, filepath);
    WAIT_INPUT;
    Tree_dump(AST_tree, "check tree");
    LOG_DUMP_GRAPH(AST_tree, "Check tree", Tree_dump_graph);


    return 1;
}
