//
// Created by IvanBrekman on 17.01.2022
//

#ifndef ASMGENH
#define ASMGENH

#include "../back.h"

struct ASMGenerateContext {
         FILE* asm_file      = NULL;

    NameTable* nametable     = NULL;
    Namespace* cur_namespace = NULL;
    StdContext* std_ctx      = NULL;
    char* func_name          = NULL;
    int is_std_func          = 0;
};

int generate_asm_code_from_tree(const FrontContext* context);
int add_standard_library(const ASMGenerateContext* context);

int processing_node        (const Node* node, ASMGenerateContext* context);
int processing_operator    (const Node* node, ASMGenerateContext* context);
int processing_special_name(const Node* node, ASMGenerateContext* context);

int get_variable_address(const Node* node, ASMGenerateContext* context);
int           print_node(const Node* node, ASMGenerateContext* context);

int check_name_type(const Node* node, ASMGenerateContext* context, const char* name, name_type type);
int     is_std_func(const Node* node, ASMGenerateContext* context);

#endif // ASMGENH
