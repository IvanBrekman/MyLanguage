//
// Created by IvanBrekman on 13.12.2021
//

#include "../../config.h"
#include "../../libs/baselib.h"

#include "front.h"
#include "Tokenizator/token.h"

int Front(const char* filepath) {
    ASSERT_IF(filepath, "invalid filepath ptr", 0);

    Tokens* tokens = get_tokens(filepath);
    tokens_dump(tokens, "Check tokens");

    return 1;
}
