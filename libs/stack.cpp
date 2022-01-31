//
// Created by ivanbrekman on 20.09.2021.
//

#include "../config.h"

#include <cmath>
#include <cassert>
#include <malloc.h>

#include "stack.h"
#include "baselib.h"

#if !defined(VALIDATE_LEVEL)
    #define VALIDATE_LEVEL NO_VALIDATE
#endif

//! Stack constructor
//! \param stack pointer to empty stack
//! \param info  pointer to structure with stack info
//! \param error pointer to value, where error will be written (default: NULL)
//! \return      1, if all is good
int Stack_ctor_(Stack* stack, StackInfo* info, int* error) {
    if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
        assert(VALID_PTR(stack) && "Invalid stack ptr");
        assert(VALID_PTR(info)  && "Invalid info ptr");
    }
    if (!VALID_PTR(stack) && VALID_PTR(error)) {
        *error = errors::ST_INVALID_PTR;
    }
    if (!VALID_PTR(info) && VALID_PTR(error)) {
        *error = errors::ST_INFO_INVALID_PTR;
    }

    stack->size     = 0;
    stack->capacity = CAP_STEP;
    stack->data     = (stack_el_t *)calloc(stack->capacity * sizeof(stack_el_t) + 2 * sizeof(long long), sizeof(char));
    *((long long*)stack->data) = CANARY;
    *((long long*)((char*)stack->data + sizeof(long long) + stack->capacity * sizeof(stack_el_t))) = CANARY;
    stack->data = (stack_el_t*)((char*)stack->data + sizeof(long long));

    stack->info_  = (StackInfo*)info;

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < stack->capacity; i++) {
            stack->data[i] = poisons::UNINITIALIZED_INT;
        }
    }

    UPDATE_STACK_HASH(stack);

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        stack_dump(*stack, "check init");
    }

    ASSERT_OK(stack, Stack, "Stack_error failed. Invalid Stack constructor");
    CHECK_SOFT_ERROR(stack, Stack, error);
    return 1;
}

//! Stack destructor
//! \param stack pointer to stack object
//! \param error pointer to value, where error will be written (default: NULL)
//! \return      1, if all is good
int Stack_dtor_(Stack* stack, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed on destructor (maybe repeat dtor call?)");
    CHECK_SOFT_ERROR(stack, Stack, error);

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < stack->size; i++) {
            stack->data[i] = (stack_el_t)poisons::FREED_ELEMENT;
        }
    }
    stack->data = (stack_el_t*)((char*)stack->data - sizeof(long long));
    free(stack->data);

    stack->data     = (stack_el_t*)poisons::FREED_PTR;
    stack->capacity = poisons::FREED_ELEMENT;
    stack->size     = poisons::FREED_ELEMENT;

    if (VALIDATE_LEVEL >= STRONG_VALIDATE) {
        stack->hash      = poisons::FREED_ELEMENT;
        stack->hash_data = poisons::FREED_ELEMENT;
        stack->hash_info = poisons::FREED_ELEMENT;
    }

    return 1;
}

//! Function to detect errors in stack
//! \param stack pointer to stack
//! \return      error code (0 if all is good)
int   Stack_error(const Stack* stack) {
    if (!VALID_PTR(stack)) {
        return errors::ST_INVALID_PTR;
    }
    if (!VALID_PTR(stack->data)) {
        return errors::ST_DATA_INVALID_PTR;
    }
    if (!VALID_PTR(stack->info_)) {
        return errors::ST_INFO_INVALID_PTR;
    }
    if (stack->size < 0) {
        return errors::INCORRECT_ST_SIZE;
    }
    if (stack->capacity < 0) {
        return errors::INCORRECT_ST_CAPACITY;
    }

    if (stack->left_canary != CANARY || stack->right_canary != CANARY) {
        return errors::DAMAGED_STACK_CANARY;
    }
    if (stack->size > stack->capacity) {
        return errors::ST_SIZE_EXCEEDED_CAPACITY;
    }

    if (VALIDATE_LEVEL >= STRONG_VALIDATE) {
        unsigned long long hash = Stack_hash_(stack);
        if (hash != stack->hash) {
            return errors::INCORRECT_ST_HASH;
        }
    }

    long long l_canary = *(long long*)((char*)stack->data - sizeof(long long));
    long long r_canary = *(long long*)((char*)stack->data + stack->capacity * sizeof(stack_el_t));
    if (l_canary != CANARY || r_canary != CANARY) {
        return errors::DAMAGED_DATA_CANARY;
    }

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        for (int i = 0; i < stack->size; i++) {
            if (stack->data[i] == poisons::UNINITIALIZED_INT || stack->data[i] == poisons::FREED_ELEMENT) {
                return errors::BAD_ST_ELEMENT;
            }
        }
    }

    if (VALIDATE_LEVEL >= STRONG_VALIDATE) {
        unsigned long long hash = Stack_hash_ptr_(stack->data, stack->capacity);
        if (hash != stack->hash_data) {
            return errors::INCORRECT_ST_DATA_HASH;
        }

        hash = Stack_hash_ptr_(stack->info_, sizeof(*stack->info_));
        if (hash != stack->hash_info) {
            return errors::INCORRECT_ST_INFO_HASH;
        }
    }

    return 0;
}

//! Function with errors description
//! \param error_code code of error
//! \return           error description
const char* Stack_error_desc(int error_code) {
    switch (error_code) {
        case 0:
            return "No error";
        case errors::NOT_ENOUGH_MEMORY:
            return "Can`t find enough dynamic memory";
        case errors::ST_INVALID_PTR:
            return "Stack has invalid pointer";
        case errors::ST_DATA_INVALID_PTR:
            return "Stack.data has invalid pointer";
        case errors::ST_INFO_INVALID_PTR:
            return "Stack.info_ has invalid pointer";
        case errors::ST_EMPTY:
            return "Stack is empty";
        case errors::INCORRECT_ST_SIZE:
            return "Stack has incorrect size";
        case errors::INCORRECT_ST_CAPACITY:
            return "Stack has incorrect capacity";
        case errors::ST_SIZE_EXCEEDED_CAPACITY:
            return "Stack size is more then stack capacity";
        case errors::DAMAGED_STACK_CANARY:
            return "Stack damaged. Something messed stack canary";
        case errors::DAMAGED_DATA_CANARY:
            return "Stack damaged. Something messed stack.data canary";
        case errors::BAD_ST_ELEMENT:
            return "Stack element is incorrect";
        case errors::INCORRECT_ST_HASH:
            return "Stack has incorrect hash. Something messed data in stack";
        case errors::INCORRECT_ST_DATA_HASH:
            return "Stack.data has incorrect hash. Something messed data in stack.data";
        case errors::INCORRECT_ST_INFO_HASH:
            return "Stack.info_ has incorrect hash. Something messed data in stack.info";
        default:
            return "Unknown error code";
    }
}

//! Function calculate stack hash
//! \param stack pointer to stack
//! \return      stack hash
unsigned long long Stack_hash_(const Stack* stack) {
    ASSERT_OK(stack, Stack, "Stack_hash_ failed verifying");

    unsigned long long hash = 0;

    for (int i = 0 ; i < (int)sizeof(*stack); i++) {
        if ((i < (char*)&stack->hash - (char*)stack) || (i >= (int)((char*)&stack->hash - (char*)stack + 3 * sizeof(stack->hash)))) {
            hash += *((char*)stack + i) * (int)pow(2, i % 10);
        }
    }

    return hash;
}

//! Function calculate pointer data hash
//! \param ptr  pointer to data
//! \param size size of data
//! \return     pointer hash
unsigned long long Stack_hash_ptr_(const void* ptr, size_t size) {
    assert(VALID_PTR(ptr) && "Invalid ptr ptr");
    
    unsigned long long hash = 0;

    for (int i = 0 ; i < (int)size; i++) {
        hash += *((char*)ptr + i) * (int)pow(2, i % 10);
    }

    return hash;
}

//! Function write value to stack
//! \param stack pointer to stack
//! \param value written value
//! \param error pointer to value, where error will be written (default: NULL)
//! \return      stack size
int push(Stack* stack, stack_el_t value, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed in the beginning of push func");
    CHECK_SOFT_ERROR(stack, Stack, error);

    if (stack->size == stack->capacity) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            LOG1(printf("stack->size(%d) == stack->capacity(%d): Capacity increase from %d to %d\n",
                   stack->size, stack->capacity, stack->capacity, stack->capacity < CAP_BORDER ? 2 * stack->capacity : stack->capacity + CAP_STEP););
        }
        change_capacity(stack, stack->capacity < CAP_BORDER ? 2 * stack->capacity : stack->capacity + CAP_STEP, error);
    }

    stack->data[stack->size++] = value;

    UPDATE_STACK_HASH(stack);
    ASSERT_OK(stack, Stack, "Stack_error failed in the end of push func");
    CHECK_SOFT_ERROR(stack, Stack, error);
    return stack->size;
}

//! Function delete top element from stack
//! \param stack pointer to stack
//! \param error pointer to value, where error will be written (default: NULL)
//! \return      deleted value (or errors::ST_EMPTY if stack is empty)
int  pop(Stack* stack, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed in the beginning of pop func");
    CHECK_SOFT_ERROR(stack, Stack, error);

    if (stack->size == 0) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            StackInfo cur_info = LOCATION(obj);
            Stack_dump_(stack, &cur_info, "Cannot pop from empty stack");
            if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {
                Stack_dump_file_(stack, &cur_info, "Cannot pop from empty stack", "log.txt");
            }
            assert(0 && "Cannot pop from empty stack");
        }
        *error = errors::ST_EMPTY;
        return errors::ST_EMPTY;
    }

    stack_el_t del_element = stack->data[stack->size - 1];
    stack->data[--stack->size] = poisons::FREED_ELEMENT;
    UPDATE_STACK_HASH(stack);

    if (stack->size >= CAP_BORDER && (stack->size + 2 * CAP_STEP) == stack->capacity) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            LOG1(printf("stack->size(%d), stack->capacity(%d): Capacity decrease from %d to %d\n",
                   stack->size, stack->capacity, stack->capacity, stack->capacity - CAP_STEP););
        }
        change_capacity(stack, stack->capacity - CAP_STEP, error);
    } else if (stack->capacity > CAP_BORDER && stack->size * 2 == CAP_BORDER) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            LOG1(printf("stack->size(%d), stack->capacity(%d): Capacity decrease from %d to %d\n",
                   stack->size, stack->capacity, stack->capacity, stack->capacity - CAP_STEP););
        }
        change_capacity(stack, stack->capacity - CAP_STEP, error);
    } else if (stack->size >= CAP_STEP && stack->size * 4 == stack->capacity) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            LOG1(printf("stack->size(%d), stack->capacity(%d): Capacity decrease from %d to %d\n",
                   stack->size, stack->capacity, stack->capacity, stack->capacity / 2););
        }
        change_capacity(stack, stack->capacity / 2, error);
    } else if (stack->size == 0 && stack->capacity > CAP_STEP) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            LOG1(printf("stack->size(%d), stack->capacity(%d): Capacity decrease from %d to %d\n",
                   stack->size, stack->capacity, stack->capacity, CAP_STEP););
        }
        change_capacity(stack, CAP_STEP, error);
    }

    UPDATE_STACK_HASH(stack);
    ASSERT_OK(stack, Stack, "Stack_error failed in the end of pop func");
    CHECK_SOFT_ERROR(stack, Stack, error);
    return del_element;
}

//! Function get stack top element
//! \param stack pointer to stack
//! \param error pointer to value, where error will be written (default: NULL)
//! \return      stack top element
stack_el_t top(const Stack* stack, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed in top func");
    CHECK_SOFT_ERROR(stack, Stack, error);

    if (stack->size == 0) {
        if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
            StackInfo cur_info = LOCATION(obj);
            Stack_dump_(stack, &cur_info, "Cannot get top element from empty stack");
            if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {
                Stack_dump_file_(stack, &cur_info, "Cannot get top element from empty stack", "log.txt");
            }
            assert(0 && "Cannot get top element from empty stack");
        }
        *error = errors::ST_EMPTY;
        return errors::ST_EMPTY;
    }

    return stack->data[stack->size - 1];
}

//! Function changes stack capacity
//! \param stack        pointer to stack
//! \param new_capacity new stack capacity
//! \param error        pointer to value, where error will be written (default: NULL)
//! \return             new stack capacity (or errors::NOT_ENOUGH_MEMORY)
int change_capacity(Stack* stack, int new_capacity, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed in the beginning change_capacity func");
    CHECK_SOFT_ERROR(stack, Stack, error);

    if (VALIDATE_LEVEL >= WEAK_VALIDATE) {
        LOG1(printf("Capacity:     %d\n"
                    "Size:         %d\n"
                    "New capacity: %d\n\n",
                    stack->capacity, stack->size, new_capacity);
        );
    }

    stack->data = (stack_el_t*)((char*)stack->data - sizeof(long long));
    stack_el_t* stack_data_ptr = (stack_el_t*)realloc(stack->data, new_capacity * sizeof(stack_el_t) + 2 * sizeof(long long));
    if (!VALID_PTR(stack_data_ptr)) {
        *error = errors::NOT_ENOUGH_MEMORY;
        return errors::NOT_ENOUGH_MEMORY;
    }

    *(long long*)((char*)stack_data_ptr + sizeof(long long) + new_capacity * sizeof(stack_el_t)) = CANARY;
    stack->data = (stack_el_t*)((char*)stack_data_ptr + sizeof(long long));

    if (VALIDATE_LEVEL >= MEDIUM_VALIDATE) {
        if (stack->capacity < new_capacity) {
            for (int i = stack->capacity; i < new_capacity; i++) {
                stack->data[i] = poisons::UNINITIALIZED_INT;
            }
        }
    }
    stack->capacity = new_capacity;

    UPDATE_STACK_HASH(stack);
    ASSERT_OK(stack, Stack, "Stack_error failed in the end of change_capacity func")
    CHECK_SOFT_ERROR(stack, Stack, error);
    return stack->capacity;
}

int  print_stack(const Stack* stack, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed in print_stack func");
    CHECK_SOFT_ERROR(stack, Stack, error);

    printf("Stack <%s> Size: %d Capacity: %d\n", stack->info_->type, stack->size, stack->capacity);
    for (int i = stack->size - 1; i >= 0; i--) {
        printf("| %2d |\n", stack->data[i]);
    }
    if (stack->size > 0) printf(" ++++ \n");

    return stack->size;
}
int  print_stack_line(const Stack* stack, const char* sep, const char* end, FILE* file, int* error) {
    ASSERT_OK(stack, Stack, "Stack_error failed in print_stack func");
    CHECK_SOFT_ERROR(stack, Stack, error);

    fprintf(file, "| ");
    for (int i = stack->size - 1; i >= 0; i--) {
        fprintf(file, "%2d", stack->data[i]);
        if (i > 0) fprintf(file, "%s", sep);
    }
    fprintf(file, " ]%s", end);

    return stack->size;
}

void Stack_dump_(const Stack* stack, const StackInfo* current_info, const char reason[]) {
    printf((ORANGE "|--------------------             Stack  Dump             --------------------|\n" NATURAL));
    PRINT_DATE(BLUE);
    printf("%s\n", reason);

    int err = Stack_error(stack);
    if (err) printf(RED "%s (%d)\n" NATURAL, Stack_error_desc(err), err);

    if (!VALID_PTR(stack)) {
        printf((RED "Cannot find Stack obj at %p\n" NATURAL), stack);
        printf((ORANGE "|--------------------COMPILATION DATE %s %s--------------------|\n" NATURAL), __DATE__, __TIME__);
        return;
    }

    printf("Stack <" PURPLE "%s" NATURAL ">[" CYAN "%p" NATURAL "] (%s) " "\"" ORANGE_UNL "%s" NATURAL "\" from %s:%d, " CYAN "%s" NATURAL " function\n",
           current_info->type,
           stack,
           (err) ? (RED "FAILED" NATURAL) : (GREEN "ok" NATURAL),
           current_info->name,
           current_info->file,
           current_info->line,
           current_info->func
           );

    if (VALID_PTR(stack->info_)) {
        if (VALID_PTR(stack->info_->name) && VALID_PTR(stack->info_->file) &&
            VALID_PTR(stack->info_->func)) {
            printf("\tStack definition: \"" ORANGE_UNL "%s" NATURAL "\" from %s:%d, " CYAN "%s" NATURAL " function\n",
                stack->info_->name, stack->info_->file,
                stack->info_->line, stack->info_->func);
        } else {
            printf("\tStack definition: " RED "cant access stack.info_ data\n" NATURAL);
        }
    } else {
        printf("\tStack definition: " RED "Invalid stack.info_ ptr\n" NATURAL);
    }

    printf("{\n");

    printf("\tLeft_canary  = %llX (%s)\n",   stack->left_canary,  stack->left_canary  == CANARY ? (GREEN "ok" NATURAL) : (RED "BAD" NATURAL));
    printf("\tRight_canary = %llX (%s)\n",   stack->right_canary, stack->right_canary == CANARY ? (GREEN "ok" NATURAL) : (RED "BAD" NATURAL));

    if (VALIDATE_LEVEL >= STRONG_VALIDATE) {
        unsigned long long hash = Stack_hash_(stack);
        printf("\tHash (saved) = %llu\n",   stack->hash);
        printf("\tHash (calc)  = %llu\n\n", hash);

        printf("\tHash data (saved) = %llu\n",   stack->hash_data);
        printf("\tHash data (calc)  = ");
        if (VALID_PTR(stack->data)) printf("%llu\n\n", Stack_hash_ptr_(stack->data, stack->capacity));
        else                                    printf(RED "cant calculate (invalid ptr)\n\n" NATURAL);

        printf("\tHash info (saved) = %llu\n",   stack->hash_info);
        printf("\tHash info (calc)  = ");
        if (VALID_PTR(stack->info_)) printf("%llu\n\n", Stack_hash_ptr_(stack->info_, sizeof(*stack->info_)));
        else                                    printf(RED "cant calculate (invalid ptr)\n\n" NATURAL);
    }

    if (stack->size > stack->capacity) {
        printf((RED "\tStack size exceeded Stack capacity\n" NATURAL));
    }

    printf("\tSize     = %d", stack->size);
    if      (stack->size == poisons::UNINITIALIZED_INT)     printf((RED " (uninitialized)" NATURAL));
    else if (stack->size == poisons::FREED_ELEMENT)         printf((RED " (freed)"         NATURAL));
    printf("\n");

    printf("\tCapacity = %d", stack->capacity);
    if      (stack->capacity == poisons::UNINITIALIZED_INT) printf((RED " (uninitialized)" NATURAL));
    else if (stack->capacity == poisons::FREED_ELEMENT)     printf((RED " (freed)"         NATURAL));
    printf("\n");

    printf("\tdata[" CYAN "%p" NATURAL "]", stack->data);

    if (VALID_PTR(stack->data)) {
        printf("\n\t{\n");

        unsigned long long* l_canary = (unsigned long long*)((char*)stack->data - sizeof(long long));
        unsigned long long* r_canary = (unsigned long long*)((char*)stack->data + stack->capacity * sizeof(stack_el_t));
        printf("\t\t Data left_canary  = %llX (%s)\n",   *l_canary, *l_canary == CANARY ? (GREEN "ok" NATURAL) : (RED "BAD" NATURAL));
        printf("\t\t Data right_canary = %llX (%s)\n\n", *r_canary, *r_canary == CANARY ? (GREEN "ok" NATURAL) : (RED "BAD" NATURAL));

        int end_index = stack->capacity - 1;
        for (; end_index >= 0; end_index--) {
            if (stack->data[end_index] != 0) {
                break;
            }
        }
        for (int i = 0; i <= end_index + 1; i++) {
            printf("\t\t%s", i < stack->size ? (BLUE "*") : " ");
            printf("[%d]", i);
            if (i < stack->size) printf(NATURAL);
            printf(" = %d", stack->data[i]);
            if     (stack->data[i] == poisons::UNINITIALIZED_INT) printf((RED " (uninitialized)" NATURAL));
            else if(stack->data[i] == poisons::FREED_ELEMENT)     printf((RED " (freed)"         NATURAL));
            printf("\n");
        }
        if (end_index != stack->capacity - 1) printf("\t\t    ...\n\t\t [%d] = %d\n", stack->capacity - 1, stack->data[stack->capacity - 1]);
        printf("    }\n");
    } else {
        printf(" - " RED "Invalid ptr\n" NATURAL);
    }

    printf("}\n");
    printf(ORANGE "|--------------------COMPILATION DATE %s %s--------------------|" NATURAL "\n", __DATE__, __TIME__);
}
void Stack_dump_file_(const Stack* stack, const StackInfo* current_info, const char reason[], const char* log_file) {
    FILE* log = fopen(log_file, "a");

    fprintf(log, "|--------------------             Stack  Dump             --------------------|\n");
    FPRINT_DATE(log);
    fprintf(log, "%s\n", reason);

    int err = Stack_error(stack);
    if (err) fprintf(log, "%s (%d)\n", Stack_error_desc(err), err);

    if (!VALID_PTR(stack)) {
        fprintf(log, "Cannot find Stack obj at %p\n"
                     "|--------------------COMPILATION DATE %s %s--------------------|\n",
                     stack, __DATE__, __TIME__);
        return;
    }

    fprintf(log, "Stack <%s>[%p] (%s) \"%s\" from %s:%d, %s function\n",
            current_info->type,
            stack,
            (err) ? "FAILED" : "ok",
            current_info->name,
            current_info->file,
            current_info->line,
            current_info->func
            );

    if (VALID_PTR(stack->info_)) {
        if (VALID_PTR(stack->info_->name) && VALID_PTR(stack->info_->file) &&
            VALID_PTR(stack->info_->func)) {
            fprintf(log, "\tStack definition: \"%s\" from %s:%d, %s function\n{\n",
                    stack->info_->name, stack->info_->file,
                    stack->info_->line, stack->info_->func
                    );
        } else {
            fprintf(log, "\tStack definition: cant access stack.info_ data\n");
        }
    } else {
        fprintf(log, "\tStack definition: Invalid stack.info_ ptr\n");
    }

    fprintf(log, "\tLeft_canary  = %llX (%s)\n", stack->left_canary,  stack->left_canary  == CANARY ? "ok" : "BAD");
    fprintf(log, "\tRight_canary = %llX (%s)\n", stack->right_canary, stack->right_canary == CANARY ? "ok" : "BAD");

    if (VALIDATE_LEVEL >= STRONG_VALIDATE) {
        unsigned long long hash = Stack_hash_(stack);
        fprintf(log, "\tHash (saved) = %llu\n",   stack->hash);
        fprintf(log, "\tHash (calc)  = %llu\n\n", hash);

        fprintf(log, "\tHash data (saved) = %llu\n",   stack->hash_data);
        fprintf(log, "\tHash data (calc)  = ");
        if (VALID_PTR(stack->data)) fprintf(log, "%llu\n\n", Stack_hash_ptr_(stack->data, stack->capacity));
        else                                    fprintf(log, "cant calculate (damaged stack data)\n\n");

        fprintf(log, "\tHash info (saved) = %llu\n",   stack->hash_info);
        fprintf(log, "\tHash info (calc)  = ");
        if (VALID_PTR(stack->info_)) fprintf(log, "%llu\n\n", Stack_hash_ptr_(stack->info_, sizeof(*stack->info_)));
        else                                    fprintf(log, "cant calculate (invalid ptr)\n\n");
    }

    if (stack->size > stack->capacity) {
        fprintf(log, "\tStack size exceeded Stack capacity\n");
    }

    fprintf(log, "\tSize     = %d", stack->size);
    if      (stack->size == poisons::UNINITIALIZED_INT)     fprintf(log, " (uninitialized)");
    else if (stack->size == poisons::FREED_ELEMENT)         fprintf(log, " (freed)");
    fprintf(log, "\n");

    fprintf(log, "\tCapacity = %d", stack->capacity);
    if      (stack->capacity == poisons::UNINITIALIZED_INT) fprintf(log, " (uninitialized)");
    else if (stack->capacity == poisons::FREED_ELEMENT)     fprintf(log, " (freed)");
    fprintf(log, "\n");

    fprintf(log, "\tdata[%p]", stack->data);

    if (VALID_PTR(stack->data)) {
        fprintf(log, "\n\t{\n");

        unsigned long long* l_canary = (unsigned long long*)((char*)stack->data - sizeof(long long));
        unsigned long long* r_canary = (unsigned long long*)((char*)stack->data + stack->capacity * sizeof(stack_el_t));
        fprintf(log, "\t\t Data left_canary  = %llX (%s)\n",   *l_canary, *l_canary == CANARY ? "ok" : "BAD");
        fprintf(log, "\t\t Data right_canary = %llX (%s)\n\n", *r_canary, *r_canary == CANARY ? "ok" : "BAD");

        for (int i = 0; i < stack->capacity; i++) {
            fprintf(log, "%*s%s[%d] = %d", 8, " ", i < stack->size ? "*" : " ", i, stack->data[i]);
            if     (stack->data[i] == poisons::UNINITIALIZED_INT) fprintf(log, " (uninitialized)");
            else if(stack->data[i] == poisons::FREED_ELEMENT)     fprintf(log, " (freed)");
            fprintf(log, "\n");
        }
        fprintf(log, "    }\n");
    } else {
        fprintf(log, " - Invalid ptr\n");
    }

    fprintf(log, "}\n"
                 "|--------------------COMPILATION DATE %s %s--------------------|\n\n",
                 __DATE__, __TIME__);

    printf((RED "Dump was written to the file (\"%s\")\n" NATURAL), log_file);
    fclose(log);
}
