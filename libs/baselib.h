//
// Created by ivanbrekman on 22.09.2021.
//

#ifndef BASELIB_H
#define BASELIB_H

#ifndef VALIDATE_LEVEL
    #define VALIDATE_LEVEL 0
#endif

#ifndef EXECUTE_WAITINGS
    #define EXECUTE_WAITINGS 0
#endif

#ifndef LOG_PRINTF
    #define LOG_PRINTF 0
#endif

#ifndef LOG_GRAPH
    #define LOG_GRAPH 0
#endif

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>

#define dbg(code)      do { printf("%s:%d\n", __FILE__, __LINE__); code } while (0)
#define LOCATION(var)  { TYPE, #var, __FILE__, __FUNCTION__, __LINE__ }
#define VALID_PTR(ptr) !isbadreadptr((const void*)(ptr))
#define WAIT_INPUT     do { if (EXECUTE_WAITINGS == 1) { printf(BLUE "Press any button...\n" NATURAL); getchar(); } } while(0)

#define COLORED_OUTPUT(str, color, file) IS_TERMINAL(file) ? (color str NATURAL) : str
#define IS_TERMINAL(file)                (file == stdin) || (file == stdout) || (file == stderr)
#define INT_ADDRESS(ptr)                 (int)((char*)(ptr) - (char*)0)

const double FLOAT_COMPARE_PRESICION = 0.0001;

const int MAX_FILEPATH_SIZE       =  50;
const int MAX_SPRINTF_STRING_SIZE = 500;

void* calloc_s(size_t __nmemb, size_t __size);

FILE* open_file(const char* filename, const char mode[]);
int  close_file(FILE* file);

/*
Default define to ASSERT_OK. Use it to customize macros for each project.
!Note!  To use this macro check that open_file function is defined
        or use another function to open file.

#define ASSERT_OK(obj, type, reason, ret) do {                                      \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE && type ## _error(obj)) {                   \
        type ## _dump(obj, reason);                                                 \
        if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                   \
            LOG_DUMP(obj, reason, type ## _dump)                                    \
        }                                                                           \
        ASSERT_IF(0, "verify failed", ret);                                         \
    } else if (type ## _error(obj)) {                                               \
        errno = type ## _error(obj);                                                \
        return ret;                                                                 \
    }                                                                               \
} while (0)

*/

#define LOG_DUMP(obj, reason, func) do {                                            \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* log = open_file("log.txt", "a");                                      \
        func(obj, reason, log);                                                     \
        close_file(log);                                                            \
    }                                                                               \
} while (0)

#define LOG_DUMP_GRAPH(obj, reason, func) do {                                      \
    if (LOG_GRAPH == 1) {                                                           \
        FILE* gr_log = open_file("log.html", "a");                                  \
        func(obj, reason, gr_log);                                                  \
        close_file(gr_log);                                                         \
    }                                                                               \
} while (0)

#define PRINT_WARNING(text) do {                                                    \
    printf(__FILE__ ":%d " ORANGE text NATURAL, __LINE__);                          \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* wlog = open_file("log.txt", "a");                                     \
        fprintf(wlog, __FILE__ ":%d " text, __LINE__);                              \
        close_file(wlog);                                                           \
    }                                                                               \
} while (0)

#define APRINT_WARNING(text, args...) do {                                          \
    printf(__FILE__ ":%d " ORANGE text NATURAL, __LINE__, args);                    \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* wlog = open_file("log.txt", "a");                                     \
        fprintf(wlog, __FILE__ ":%d " text, __LINE__, args);                        \
        close_file(wlog);                                                           \
    }                                                                               \
} while (0)

#define SPR_SYSTEM(format...) do {                                                  \
    char* command = (char*) calloc_s(MAX_SPRINTF_STRING_SIZE, sizeof(char));        \
    sprintf(command, format);                                                       \
                                                                                    \
    system(command);                                                                \
                                                                                    \
    FREE_PTR(command, char);                                                        \
} while (0)

#define SPR_FPUTS(file, format...) do {                                             \
    char* string = (char*) calloc_s(MAX_SPRINTF_STRING_SIZE, sizeof(char));         \
    sprintf(string, format);                                                        \
                                                                                    \
    fputs(string, file);                                                            \
                                                                                    \
    FREE_PTR(string, char);                                                         \
} while (0)

#define ASSERT_IF(cond, text, ret) do {                                             \
    assert((cond) && text);                                                         \
    if (!(cond)) {                                                                  \
        PRINT_WARNING(text "\n");                                                   \
        errno = -1;                                                                 \
        return ret;                                                                 \
    }                                                                               \
} while(0)

#define FREE_PTR(ptr, type) do {                \
    free((ptr));                                \
    (ptr) = (type*)poisons::FREED_PTR;          \
} while (0)
#define LOG1(code) do {                         \
    if (LOG_PRINTF >= 1) {                      \
        code                                    \
    }                                           \
} while (0)
#define LOG2(code) do {                         \
    if (LOG_PRINTF >= 2) {                      \
        code                                    \
    }                                           \
} while (0)

// Colors----------------------------------------------------------------------
#define BLACK       "\033[1;30m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define ORANGE      "\033[1;33m"
#define BLUE        "\033[1;34m"
#define PURPLE      "\033[1;35m"
#define CYAN        "\033[1;36m"
#define GRAY        "\033[1;37m"

#define BLACK_UNL   "\033[4;30m"
#define RED_UNL     "\033[4;31m"
#define GREEN_UNL   "\033[4;32m"
#define ORANGE_UNL  "\033[4;33m"
#define BLUE_UNL    "\033[4;34m"
#define PURPLE_UNL  "\033[4;35m"
#define CYAN_UNL    "\033[4;36m"
#define GRAY_UNL    "\033[4;37m"

#define NATURAL     "\033[0m"
// ----------------------------------------------------------------------------

enum validate_level {
    NO_VALIDATE      = 0, // No checks in program
    WEAK_VALIDATE    = 1, // Checks only fields with  O(1) complexity
    MEDIUM_VALIDATE  = 2, // Checks filed, which need O(n) complexity
    STRONG_VALIDATE  = 3, // All checks (hash and others)
    HIGHEST_VALIDATE = 4  // Error will write in log file
};

enum poisons {
    UNINITIALIZED_PTR =  6,
    UNINITIALIZED_INT = -1 * (0xBAD666),

    FREED_ELEMENT     = -1 * (0xBAD667),
    FREED_PTR         = 12
};

#define PRINT_DATE(color) do {                                      \
    char* date = (char*) calloc_s(40, sizeof(char));                \
    printf((color "Time: %s\n" NATURAL), datetime(date));           \
    FREE_PTR(date, char);                                           \
} while (0)
#define FPRINT_DATE(file) do {                                      \
    char* date = (char*) calloc_s(40, sizeof(char));                \
    fprintf(file, "Time: %s\n", datetime(date));                    \
    FREE_PTR(date, char);                                           \
} while (0)

int      file_size(const char* filename);
char* get_raw_text(const char* filename);
char* delete_spaces(      char* string);

int isbadreadptr(const void* ptr);
char* datetime(char* calendar_date);

int is_integer(double number);
int is_number(char* string);
int digits_number(int number, int radix=10);
int   extract_bit(int number, int bit);

char* bin4(int number);
char* to_string(int number);

#endif //BASELIB_H
