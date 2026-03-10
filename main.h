/***********************************************************************
 *  File name   : main.h
 *  Description : Header file for the Lexical Analyzer Project.
 *                Declares data structures and function prototypes
 *                required for lexical tokenization and statistics tracking.
 *
 *                Responsibilities:
 *                - Define TokenStats structure for counting token categories
 *                - Define LexerState structure for per-file lexer state
 *                - Declare tokenize_line() for per-line lexical analysis
 *                - Declare lexer_state_init() and lexer_state_free()
 *
 *                Structures:
 *                - TokenStats
 *                - LexerState
 *
 *                Functions:
 *                - lexer_state_init()
 *                - lexer_state_free()
 *                - tokenize_line()
 ***********************************************************************/
#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*----------------------------------------------------------------------
 *  Structure: TokenStats
 *  Description:
 *      Holds counters for each token category identified
 *      during lexical analysis of the input source code.
 *--------------------------------------------------------------------*/
typedef struct
{
    int keyword;
    int identifiers;
    int operators;
    int integers;
    int real_numbers;
    int string_literals;
    int character_literals;
    int delimiters;
    int preprocessor;
    int hex_octal;
    int single_line;
    int multi_line;
} TokenStats;

/*----------------------------------------------------------------------
 *  Structure: LexerState
 *  Description:
 *      Holds all mutable state required by tokenize_line() across
 *      successive calls for the same source file.
 *
 *  Fields:
 *      buffer_multi  - heap-allocated buffer for multi-line comments
 *                      and string literals that span multiple lines
 *      buf_capacity  - current allocated size of buffer_multi
 *      m_index       - next write position in buffer_multi (comments)
 *      s_index       - next write position in buffer_multi (strings)
 *      multi_flag    - 1 if currently inside a multi-line comment
 *      string_flag   - 1 if currently inside a multi-line string literal
 *--------------------------------------------------------------------*/
typedef struct
{
    char *buffer_multi;
    int   buf_capacity;
    int   m_index;
    int   s_index;
    int   multi_flag;
    int   string_flag;
} LexerState;

/*----------------------------------------------------------------------
 *  Function Prototypes
 *--------------------------------------------------------------------*/
/* Initialise a LexerState before processing a new source file */
void lexer_state_init(LexerState *ls);

/* Release heap memory inside a LexerState after processing is done */
void lexer_state_free(LexerState *ls);

/* Tokenize one line of source code and update statistics */
void tokenize_line(char *line, FILE *fptr_output, TokenStats *token_stats,
                   LexerState *ls);

#endif
