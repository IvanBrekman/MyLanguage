//
// Created by ivanbrekman on 26.08.2021.
//

#ifndef FILE_FUNCS_H
#define FILE_FUNCS_H

#include <cstdio>

struct String {
    char* ptr;
    size_t   len;
};

struct Text {
    char* data;
    size_t data_size;

    String* text;
    size_t lines;
};

int replace(char* string, size_t size, char old_symbol, char new_symbol, int n_replace=-1);

void free_text(Text* data);
void free_string(String* string);

void print_text(const Text* data, const char* sep=", ", const char* end="\n");
void print_strings(const char** array, size_t size, const char* sep=", ", const char* end="\n");

int  load_string_pointers(Text* text, int skip_empty_strings=0, int skip_first_last_spaces=0);
Text convert_to_text(const char** strings, int n_strings);

int  get_file_size   (const char* filename);
long file_last_change(const char* filename);

FILE* openf(const char* filename, const char mode[]);
Text get_text_from_file(const char* filename, int skip_empty_strings=0, int skip_first_last_spaces=0);

int    write_text_to_file(const char* filename, const char mode[], const Text* data, const char* text_sep="\n", const char* text_end="\n");
int  write_buffer_to_file(const char* filename, const char mode[], const Text* data, const char*  buf_sep="\n", const char*  buf_end="\n");
int write_strings_to_file(const char* filename, const char mode[], const char** data, int n_strings, const char* str_sep="\n", const char* str_end="\n");

#endif //FILE_FUNCS_H
