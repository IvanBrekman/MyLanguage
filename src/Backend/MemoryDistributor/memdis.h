//
// Created by IvanBrekman on 17.01.2022
//

#ifndef MEMDISH
#define MEMDISH

#include "../back.h"

struct AllocateContext {
    int namespaces = -1;
    int variables  = -1;
};

AllocateContext* allocate_memory(const NameTable* nametable);
int allocate_for_namespace(const Namespace* nmsp, int mem_shift);

#endif // MEMDISH
