/***********************************************************************
 *  File name   : lexer.c
 *  Description : Implements lexical token classification and line tokenization
 *                logic for the Lexical Analyzer Project. This module identifies
 *                tokens such as keywords, identifiers, literals, operators,
 *                and comments, updating token statistics accordingly.
 *
 *                Responsibilities:
 *                - Validate and classify tokens
 *                - Handle multi-line and single-line comments
 *                - Track strings, character literals, and preprocessor directives
 *
 *                Functions:
 *                - is_integer()
 *                - is_delim()
 *                - is_hex_octal()
 *                - is_keyword()
 *                - is_operator()
 *                - is_real_number()
 *                - is_pre_processor()
 *                - is_identifier()
 *                - lexer_state_init()
 *                - tokenize_line()
 ***********************************************************************/

#include "main.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/*----------------------------------------------------------------------
 *  Constants
 *--------------------------------------------------------------------*/
#define MULTI_BUF_INIT 512   /* initial capacity of the multi-line buffer  */

/*----------------------------------------------------------------------
 *  Function Declarations
 *--------------------------------------------------------------------*/
int  is_integer(char *);
int  is_delim(char *);
int  is_hex_octal(char *);
int  is_keyword(char *);
int  is_operator(char *);
int  is_real_number(char *);
int  is_pre_processor(char *);
int  is_identifier(char *);

/*----------------------------------------------------------------------
 *  LexerState – caller-owned struct that replaces all static locals.
 *  Initialise once per source file with lexer_state_init(); free
 *  the internal buffer with lexer_state_free() when done.
 *--------------------------------------------------------------------*/
void lexer_state_init(LexerState *ls)
{
    ls->multi_flag   = 0;
    ls->string_flag  = 0;
    ls->m_index      = 0;
    ls->s_index      = 0;
    ls->buf_capacity = MULTI_BUF_INIT;
    ls->buffer_multi = (char *)malloc(ls->buf_capacity);
    if (ls->buffer_multi)
        ls->buffer_multi[0] = '\0';
}

void lexer_state_free(LexerState *ls)
{
    free(ls->buffer_multi);
    ls->buffer_multi = NULL;
}

/*----------------------------------------------------------------------
 *  Internal helper: append one character to ls->buffer_multi, growing
 *  the buffer if necessary.  Returns 0 on allocation failure.
 *--------------------------------------------------------------------*/
static int buf_append(LexerState *ls, char c)
{
    /* Reserve one byte for the NUL terminator */
    if (ls->m_index + 1 >= ls->buf_capacity)
    {
        int new_cap = ls->buf_capacity * 2;
        char *tmp   = (char *)realloc(ls->buffer_multi, new_cap);
        if (!tmp)
            return 0;
        ls->buffer_multi  = tmp;
        ls->buf_capacity  = new_cap;
    }
    ls->buffer_multi[ls->m_index++] = c;
    return 1;
}

/*----------------------------------------------------------------------
 *  Check if token is an integer constant
 *--------------------------------------------------------------------*/
int is_integer(char *str)
{
    int i = 0;

    if (str == NULL || str[0] == '\0')
        return 0;

    if (str[i] == '+' || str[i] == '-')
        i++;

    if (str[i] == '\0')
        return 0;

    while (str[i] != '\0')
    {
        if (!isdigit((unsigned char)str[i]))
            return 0;
        i++;
    }
    return 1;
}

/*----------------------------------------------------------------------
 *  Check if token is a delimiter
 *--------------------------------------------------------------------*/
int is_delim(char *str)
{
    const char *delim[] = {",", ";", "(", ")", "[", "]", "{", "}", ":"};
    int len = (int)(sizeof(delim) / sizeof(delim[0]));

    for (int i = 0; i < len; i++)
    {
        if (strcmp(delim[i], str) == 0)
            return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *  Check if token is a hexadecimal or octal constant
 *--------------------------------------------------------------------*/
int is_hex_octal(char *str)
{
    if (str == NULL || str[0] == '\0')
        return 0;

    /* Hexadecimal: 0x... or 0X... */
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        if (str[2] == '\0')          /* bare "0x" is not valid */
            return 0;
        for (int i = 2; str[i] != '\0'; i++)
        {
            if (!isxdigit((unsigned char)str[i]))
                return 0;
        }
        return 1;
    }

    /* Octal: leading 0 followed by octal digits */
    if (str[0] == '0' && str[1] != '\0')
    {
        for (int i = 1; str[i] != '\0'; i++)
        {
            if (str[i] < '0' || str[i] > '7')
                return 0;
        }
        return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------
 *  Check if token is a C keyword
 *--------------------------------------------------------------------*/
int is_keyword(char *str)
{
    const char *keywords[] = {
        "char", "int", "float", "double", "signed", "unsigned", "short",
        "long", "const", "volatile", "for", "while", "do", "goto", "break",
        "continue", "if", "else", "switch", "case", "default", "auto",
        "register", "static", "extern", "struct", "union", "enum",
        "typedef", "void", "return", "sizeof"
    };

    int len = (int)(sizeof(keywords) / sizeof(keywords[0]));
    for (int i = 0; i < len; i++)
    {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *  Check if token is an operator
 *--------------------------------------------------------------------*/
int is_operator(char *str)
{
    const char *operators[] = {
        /* arithmetic */
        "+", "-", "*", "/", "%",
        /* relational */
        "==", "!=", "<", ">", "<=", ">=",
        /* assignment */
        "=",
        /* compound assignment – were missing before */
        "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
        /* increment / decrement */
        "++", "--",
        /* logical */
        "&&", "||", "!",
        /* bitwise */
        "&", "|", "^", "~", "<<", ">>",
        /* member access */
        ".", "->",
        /* ternary */
        "?"
    };

    int len = (int)(sizeof(operators) / sizeof(operators[0]));
    for (int i = 0; i < len; i++)
    {
        if (strcmp(str, operators[i]) == 0)
            return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *  Check if token is a real number constant
 *--------------------------------------------------------------------*/
int is_real_number(char *str)
{
    int i = 0, flag = 0, digit_count = 0;

    if (str == NULL || str[0] == '\0') 
        return 0;

    if (str[0] == '-' || str[0] == '+')
        i++;

    while (str[i] != '\0')
    {
        if (str[i] == '.')
        {
            flag++;
            if (flag > 1)
                return 0;
        }
        else if (isdigit((unsigned char)str[i]))
            digit_count++;
        else
            return 0;

        i++;
    }

    return (flag == 1 && digit_count > 0);
}

/*----------------------------------------------------------------------
 *  Check if token is a preprocessor directive
 *--------------------------------------------------------------------*/
int is_pre_processor(char *str)
{
    if (str == NULL || str[0] != '#')
        return 0;

    const char *preprocessors[] = {
        "#include", "#define", "#undef", "#ifdef", "#ifndef",
        "#endif", "#if", "#else", "#elif", "#error", "#pragma", "#line"
    };

    int len = (int)(sizeof(preprocessors) / sizeof(preprocessors[0]));

    for (int i = 0; i < len; i++)
    {
        size_t klen = strlen(preprocessors[i]);
        if (strncmp(str, preprocessors[i], klen) == 0)
            return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *  Check if token is a valid identifier
 *--------------------------------------------------------------------*/
int is_identifier(char *str)
{
    if (str == NULL || str[0] == '\0')
        return 0;

    if (!isalpha((unsigned char)str[0]) && str[0] != '_')
        return 0;

    for (int i = 1; str[i] != '\0'; i++)
    {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_')
            return 0;
    }

    return 1;
}

/*----------------------------------------------------------------------
 *  Tokenize and classify tokens within a line of source code.
 *--------------------------------------------------------------------*/
void tokenize_line(char *line, FILE *fptr_output, TokenStats *token_stats,
                   LexerState *ls)
{
    int  i = 0;
    char token[1024];

    while (line[i] != '\0')
    {
        /* ── whitespace ─────────────────────────────────────────── */
        if (isspace((unsigned char)line[i]))
        {
            i++;
            continue;
        }

        /* ── single-character delimiter ──────────────────────────── */
        char delim_buf[2] = {line[i], '\0'};
        if (is_delim(delim_buf))
        {
            fprintf(fptr_output, "%c is a delimiter\n", line[i]);
            token_stats->delimiters++;
            i++;
            continue;
        }

        /* ── preprocessor directive (whole rest of line) ─────────── */
        if (line[i] == '#')
        {
            int j = 0;
            while (line[i] != '\0' && j < (int)sizeof(token) - 1)
                token[j++] = line[i++];
            token[j] = '\0';

            if (is_pre_processor(token))
            {
                fprintf(fptr_output, "%s is a preprocessor directive\n", token);
                token_stats->preprocessor++;
            }
            continue;
        }

        /* ── string literal (possibly spanning multiple calls) ───── */
        if (line[i] == '"' || ls->string_flag == 1)
        {
            if (ls->s_index < ls->buf_capacity - 1)
                ls->buffer_multi[ls->s_index++] = line[i++];
            else
                i++;   /* buffer full – skip character rather than overflow */

            while (line[i] != '"' && line[i] != '\0')
            {
                if (ls->s_index < ls->buf_capacity - 1)
                    ls->buffer_multi[ls->s_index++] = line[i];
                i++;
            }

            if (line[i] == '\0')
            {
                ls->string_flag = 1;
                ls->buffer_multi[ls->s_index] = '\0';
                return;
            }

            /* closing '"' */
            if (ls->s_index < ls->buf_capacity - 1)
                ls->buffer_multi[ls->s_index++] = line[i++];
            else
                i++;

            ls->buffer_multi[ls->s_index] = '\0';
            fprintf(fptr_output, "%s is a string literal\n", ls->buffer_multi);
            ls->s_index     = 0;
            ls->string_flag = 0;
            token_stats->string_literals++;
            continue;
        }

        /* ── multi-line comment (possibly spanning multiple calls) ── */
        if ((line[i] == '/' && line[i + 1] == '*') || ls->multi_flag == 1)
        {
            while (line[i] != '\0' &&
                   !(line[i] == '*' && line[i + 1] == '/'))
            {
                buf_append(ls, line[i++]);
            }

            if (line[i] == '\0')
            {
                ls->multi_flag = 1;
                ls->buffer_multi[ls->m_index] = '\0';
                return;
            }

            /* consume closing "*/" */
            buf_append(ls, line[i++]);
            buf_append(ls, line[i++]);
            ls->buffer_multi[ls->m_index] = '\0';
            fprintf(fptr_output, "%s is a multi-line comment\n",
                    ls->buffer_multi);
            token_stats->multi_line++;
            ls->m_index    = 0;
            ls->multi_flag = 0;
            continue;
        }

        /* ── single-line comment ─────────────────────────────────── */
        if (line[i] == '/' && line[i + 1] == '/')
        {
            int j = 0;
            while (line[i] != '\0' && j < (int)sizeof(token) - 1)
                token[j++] = line[i++];
            token[j] = '\0';
            fprintf(fptr_output, "%s is a single line comment\n", token);
            token_stats->single_line++;
            return;
        }

        /* ── keyword or identifier ───────────────────────────────── */
        if (isalpha((unsigned char)line[i]) || line[i] == '_')
        {
            int j = 0;
            while ((isalnum((unsigned char)line[i]) || line[i] == '_') &&
                   j < (int)sizeof(token) - 1)
                token[j++] = line[i++];
            token[j] = '\0';

            if (is_keyword(token))
            {
                fprintf(fptr_output, "%s is a keyword\n", token);
                token_stats->keyword++;
            }
            else if (is_identifier(token))
            {
                fprintf(fptr_output, "%s is an identifier\n", token);
                token_stats->identifiers++;
            }
            continue;
        }

        /* ── numeric literal (integer, real, hex, octal) ────────────
         *  They are handled by the operator block below.
         *  is_real_number() and is_integer() already accept an
         *  optional leading sign internally, but we only enter this
         *  branch on an unambiguous digit start.
         * ─────────────────────────────────────────────────────────── */
        if (isdigit((unsigned char)line[i]))
        {
            int j = 0;
            while (line[i] != '\0' && j < (int)sizeof(token) - 1 &&
                   (isalnum((unsigned char)line[i]) ||
                    line[i] == '.' ||
                    line[i] == 'x' ||
                    line[i] == 'X'))
                token[j++] = line[i++];
            token[j] = '\0';

            if (is_real_number(token))
            {
                fprintf(fptr_output, "%s is a real number\n", token);
                token_stats->real_numbers++;
            }
            else if (is_hex_octal(token))
            {
                fprintf(fptr_output, "%s is a hex or octal number\n", token);
                token_stats->hex_octal++;
            }
            else if (is_integer(token))
            {
                fprintf(fptr_output, "%s is an integer\n", token);
                token_stats->integers++;
            }
            continue;
        }

        /* ── character literal ───────────────────────────────────── */
        if (line[i] == '\'')
        {
            int j = 0;
            token[j++] = line[i++];

            if (line[i] == '\\')
            {
                token[j++] = line[i++];
                if (line[i] != '\0')
                    token[j++] = line[i++];
            }
            else if (line[i] != '\0')
                token[j++] = line[i++];

            if (line[i] == '\'')
                token[j++] = line[i++];

            token[j] = '\0';
            fprintf(fptr_output, "%s is a character literal\n", token);
            token_stats->character_literals++;
            continue;
        }

        /* ── operator ────────────────────────────────────────────────
         *  Try three-character, two-character, then one-character
         *  matches so that e.g. ">>=" is preferred over ">>" or ">".
         * ─────────────────────────────────────────────────────────── */
        if (strchr("=<>!&|+-*/%.~^?", line[i]))
        {
            /* try 3-char first (e.g. >>=, <<=) */
            if (line[i + 1] != '\0' && line[i + 2] != '\0')
            {
                char op3[4] = {line[i], line[i + 1], line[i + 2], '\0'};
                if (is_operator(op3))
                {
                    fprintf(fptr_output, "%s is an operator\n", op3);
                    token_stats->operators++;
                    i += 3;
                    continue;
                }
            }

            /* try 2-char */
            if (line[i + 1] != '\0')
            {
                char op2[3] = {line[i], line[i + 1], '\0'};
                if (is_operator(op2))
                {
                    fprintf(fptr_output, "%s is an operator\n", op2);
                    token_stats->operators++;
                    i += 2;
                    continue;
                }
            }

            /* try 1-char */
            char op1[2] = {line[i], '\0'};
            if (is_operator(op1))
            {
                fprintf(fptr_output, "%s is an operator\n", op1);
                token_stats->operators++;
                i++;
                continue;
            }
        }

        /* ── unknown character ───────────────────────────────────── */
        fprintf(fptr_output, "%c is unknown\n", line[i]);
        i++;
    }
}
