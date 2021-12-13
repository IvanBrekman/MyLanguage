//
// Created by ivanbrekman on 20.09.2021.
//

#ifndef SIMPLEPROCESSOR_STACK_H
#define SIMPLEPROCESSOR_STACK_H

#include "../config.h"

#include <malloc.h>
#include "baselib.h"

#define CANARY 0xDEADA2EA // DEAD AREA
#define CAP_STEP     8
#define CAP_BORDER 256

#define TYPE "int"
typedef int stack_el_t;

#define UPDATE_STACK_HASH(stack) {                                                  \
    if (VALIDATE_LEVEL >= STRONG_VALIDATE) {                                        \
        stack->hash      = Stack_hash_(stack);                                      \
        stack->hash_data = Stack_hash_ptr_(stack->data, stack->capacity);           \
        stack->hash_info = Stack_hash_ptr_(stack->info_, sizeof(*stack->info_));    \
    }                                                                               \
}
#define ASSERT_OK(obj, type, reason) {                                              \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE && type ## _error(obj)) {                   \
        StackInfo cur_info = LOCATION(obj);                                         \
        type ## _dump_(obj, &cur_info, reason);                                     \
        if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                   \
            type ## _dump_file_(obj, &cur_info, reason, "log.txt");                 \
        }                                                                           \
        assert(0 && "verify failed");                                               \
    }                                                                               \
}
#define CHECK_SOFT_ERROR(obj, type, error) {                                        \
    if (VALIDATE_LEVEL >= NO_VALIDATE) {                                            \
        int err = type ## _error(obj);                                              \
        if (err && VALID_PTR(error)) *(error) = err;                                \
        if (err) return err;                                                        \
    }                                                                               \
}
#define stack_ctor(st) {                                                            \
    StackInfo* info = (StackInfo*)calloc(1, sizeof(StackInfo));                     \
    if (VALID_PTR(info)) {                                                          \
        *info = LOCATION(st);                                                       \
    }                                                                               \
    Stack_ctor_(&(st), info);                                                       \
}
#define stack_dump(st, reason) {                                                    \
    StackInfo* st_info = (StackInfo*)calloc(1, sizeof(StackInfo));                     \
    if (VALID_PTR(st_info)) {                                                          \
        *st_info = LOCATION(st);                                                       \
    }                                                                               \
    Stack_dump_(&(st), st_info, reason);                                               \
}

#include "baselib.h"

enum errors {
    NOT_ENOUGH_MEMORY         =  -1,

    ST_INVALID_PTR            =  -3,
    ST_DATA_INVALID_PTR       =  -4,
    ST_INFO_INVALID_PTR       =  -5,
    ST_EMPTY                  =  -6,
    INCORRECT_ST_SIZE         =  -7,
    INCORRECT_ST_CAPACITY     =  -8,
    ST_SIZE_EXCEEDED_CAPACITY =  -9,

    DAMAGED_STACK_CANARY      = -10,
    DAMAGED_DATA_CANARY       = -11,
    BAD_ST_ELEMENT            = -12,

    INCORRECT_ST_HASH         = -13,
    INCORRECT_ST_DATA_HASH    = -14,
    INCORRECT_ST_INFO_HASH    = -15,
};

struct Stack {
    const unsigned long long  left_canary = CANARY;

    stack_el_t* data = (stack_el_t*)poisons::UNINITIALIZED_PTR;
    int capacity     = poisons::UNINITIALIZED_INT;
    int size         = poisons::UNINITIALIZED_INT;

    unsigned long long hash      = poisons::UNINITIALIZED_INT;
    unsigned long long hash_data = poisons::UNINITIALIZED_INT;
    unsigned long long hash_info = poisons::UNINITIALIZED_INT;

    const struct StackInfo* info_ = (StackInfo*)poisons::UNINITIALIZED_PTR;

    const unsigned long long right_canary = CANARY;
};

struct StackInfo {
    const char* type;
    const char* name;
    const char* file;
    const char* func;
    int line;
};

int Stack_ctor_(Stack* stack, StackInfo* info, int* error=NULL);
int Stack_dtor_(Stack* stack, int* error=NULL);

int   Stack_error(const Stack* stack);
const char* Stack_error_desc(int error_code);

unsigned long long Stack_hash_(const Stack* stack);
unsigned long long Stack_hash_ptr_(const void* ptr, size_t size);

int       push(Stack* stack, stack_el_t value, int* error=NULL);
int        pop(Stack* stack, int* error=NULL);
stack_el_t top(const Stack* stack, int* error=NULL);

int change_capacity(Stack* stack, int new_capacity, int* error=NULL);

int  print_stack     (const Stack* stack, int* error=NULL);
int  print_stack_line(const Stack* stack, const char* sep=", ", const char* end="\n", FILE* file=stdout, int* error=NULL);
void Stack_dump_     (const Stack* stack, const StackInfo* current_info, const char reason[]);
void Stack_dump_file_(const Stack* stack, const StackInfo* current_info, const char reason[], const char* log_file);

#endif //SIMPLEPROCESSOR_STACK_H
