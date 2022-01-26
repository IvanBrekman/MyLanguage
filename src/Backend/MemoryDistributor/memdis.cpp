//
// Created by IvanBrekman on 17.01.2022
//

#include "memdis.h"

AllocateContext* allocate_memory(const NameTable* nametable) {
    ASSERT_IF(VALID_PTR(nametable), "Invalid nametable ptr", NULL);

    LOG1(printf("Allocating memory for variables...\n"););

    AllocateContext* ctx = NEW_PTR(AllocateContext, 1);
    ctx->namespaces = 1;

    LOG2(printf("Allocate global namespace.\n"););
    int mem_shift = allocate_for_namespace(&nametable->global, 0);

    for (int i = 0; i < nametable->locals_amount; i++) {
        LOG2(printf("Allocate %d local namespace.\n", i + 1););
        mem_shift = allocate_for_namespace(&nametable->locals[i], mem_shift);
    }
    ctx->variables = mem_shift;

    LOG1(printf("Successful allocating.\n\n"););
    return ctx;
}

int allocate_for_namespace(const Namespace* nmsp, int mem_shift) {
    ASSERT_IF(VALID_PTR(nmsp), "Invalid nmsp ptr", -1);

    for (int i = 0; i < nmsp->size; i++) {
        Name* cur_name = &nmsp->names[i];

        if (cur_name->type == name_type::VARIABLE) {
            cur_name->address = CONST_MEMORY_START + mem_shift++;
            LOGN(3, printf("    Allocate memory for '%s' variable. Current address: %d\n", cur_name->name, cur_name->address););
        }
    }

    return mem_shift;
}
