//
// Created by IvanBrekman on 19.12.2021
//

#ifndef CONSTANTSH
#define CONSTANTSH

// Backend/AssemblerGenerator/asm_gen.h------------------------------------------
const int CONST_MEMORY_START     = 10;
const int LOCALS_PER_STACK_FRAME = 20;
const int ASM_COMMENTS_START     = 48;

static const char* ASM_CODE_FILE = "logs/processor_work/asm_code.txt";
// ----------------------------------------------------------------------------

// Backend/Processor-----------------------------------------------------------
const int  PRINTED_RAM_BLOCKS    = 3;
const char ASM_COMMENT_SYMBOL    = ';';

static const char* HOME_DIR = "src/Backend/Processor";
// ----------------------------------------------------------------------------


/* FRONTEND CONSTANTS *///+++++++++++++++++++++++++++++++++++++++++++++++++++++

// Frontend/front.h------------------------------------------------------------
const int MAX_LEXEMS_AMOUNT = 1000;
const int MAX_NAMES_AMOUNT  = 100;
const int MAX_NAME_SIZE     = 40;
// ----------------------------------------------------------------------------

// Frontend/SyntaxAnalyzer/anal.h----------------------------------------------
const int MAX_LOCAL_NAMESPACES_AMOUNT = 100;

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
    ";",  "(",  ")",  "+",  "-",  "*",  "/",  "=", "<", ">", "$",
};
const char      LINE_COMMENT_SYMBOL = '#';
const char MULTILINE_COMMENT_SYMBOL = '@';
const int OPERATORS_AMOUNT = sizeof(OPERATORS) / sizeof(OPERATORS[0]);
// ----------------------------------------------------------------------------

/* END BLOCK *///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // CONSTANTSH
