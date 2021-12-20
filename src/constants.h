//
// Created by IvanBrekman on 19.12.2021
//

#ifndef CONSTANTSH
#define CONSTANTSH

// Frontend/front.h------------------------------------------------------------
const int MAX_LEXEMS_AMOUNT = 1000;
const int MAX_NAMES_AMOUNT  = 100;
const int MAX_NAME_SIZE     = 40;
// ----------------------------------------------------------------------------

// Frontend/SyntaxAnalyzer/anal.h----------------------------------------------
static const char*  OPEN_BRACKET = "(";
static const char* CLOSE_BRACKET = ")";
static const char*   START_BLOCK = "{";
static const char*     END_BLOCK = "}";
static const char* END_STATEMENT = ";";
static const char*  FINISH_TOKEN = "$";

static const char* RESERVED_NAMES[] = {
    "if", "else", "elif", "while", "for", "def", "func", "print"
};
const int RESERVED_NAMES_AMOUNT = sizeof(RESERVED_NAMES) / sizeof(RESERVED_NAMES[0]);

const int     REQUIRED = 1;
const int NON_REQUIRED = 0;
// ----------------------------------------------------------------------------

// Frontend/Tokenizer/token.h--------------------------------------------------
static const char* OPERATORS[] = {
    "==", "!=", "<=", ">=",
    ";",  "(",  ")",  "+",  "-",  "*",  "/",  "=", "<", ">", "$"
};
const int OPERATORS_AMOUNT = sizeof(OPERATORS) / sizeof(OPERATORS[0]);
// ----------------------------------------------------------------------------

#endif // CONSTANTSH
