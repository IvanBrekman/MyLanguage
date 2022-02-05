//
// Created by IvanBrekman on 17.01.2022
//

#ifndef ASMGENH
#define ASMGENH

#include "../back.h"

int generate_asm_code_from_tree(const FrontContext* context);
int fill_variables_relative_address(NameTable* nametable);
int add_standard_library(ASMGenerateContext* context);
int add_user_functions(const Node* node, ASMGenerateContext* context);

int processing_node        (const Node* node, ASMGenerateContext* context);
int processing_operator    (const Node* node, ASMGenerateContext* context);
int processing_special_name(const Node* node, ASMGenerateContext* context);
int processing_func_node   (const Node* node, ASMGenerateContext* context);
int processing_if_node     (const Node* node, ASMGenerateContext* context);
int processing_while_node  (const Node* node, ASMGenerateContext* context);

int get_variable_address(const Node* node, ASMGenerateContext* context);
int           print_node(const Node* node, ASMGenerateContext* context);

Name*         get_name_obj(const Node* node, ASMGenerateContext* context, const char* name);
int        check_name_type(const Node* node, ASMGenerateContext* context, const char* name, name_type type);
int            is_std_func(const Node* node, ASMGenerateContext* context);
int check_result_usability(const Node* node, ASMGenerateContext* context);

#endif // ASMGENH
