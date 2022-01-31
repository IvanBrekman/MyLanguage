//
// Created by IvanBrekman on 13.12.2021
//

#include "back.h"
#include "AssemblerGenerator/asm_gen.h"

BackContext* Back(const FrontContext* context) {
    ASSERT_IF(VALID_PTR(context), "Invalid context ptr", NULL);

    LOG1(printf(ORANGE "\n++++++++++ BACKEND START WORKING ++++++++++\n\n\n" NATURAL););

    BackContext* ctx = NEW_PTR(BackContext, 1);

    int gen = generate_asm_code_from_tree(context);
    ctx->result = gen;
    WAIT_INPUT;

    LOG1(printf(ORANGE "\n\n++++++++++ BACKEND FINISH WORKING ++++++++++\n" NATURAL););
    return ctx;
}
