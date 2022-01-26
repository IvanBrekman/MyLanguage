//
// Created by IvanBrekman on 13.12.2021
//

#include "config.h"
#include "libs/baselib.h"

#include "front.h"
#include "Tokenizer/token.h"

FrontContext* Front(const char* filepath) {
    ASSERT_IF(filepath, "invalid filepath ptr", NULL);

    LOG1(printf(ORANGE "++++++++++ FRONTEND START WORKING ++++++++++\n\n\n" NATURAL););

    Tokens* tokens = get_tokens(filepath);
    LOGN(3, tokens_dump(tokens, "Check tokens"););
    WAIT_INPUT;

    FrontContext* front_data = build_ast_tree(tokens, filepath);
    WAIT_INPUT;

    LOGN(3, Tree_dump(front_data->AST_tree, "check tree"););
    LOG_DUMP_GRAPH(front_data->AST_tree, "Check tree", Tree_dump_graph);

    LOG1(printf(ORANGE "\n\n++++++++++ FRONTEND FINISH WORKING ++++++++++\n" NATURAL););
    return front_data;
}
