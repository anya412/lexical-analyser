/***********************************************************************
 *  File name   : main.c
 *  Description : Entry point for the Lexical Analyzer Project.
 *                Reads an input C source file, tokenizes it line by line,
 *                and generates a summary report containing token counts.
 
 *                Responsibilities:
 *                - Open input and output files
 *                - Read source code line by line
 *                - Tokenize each line using tokenize_line()
 *                - Track statistics for various token categories
 *                - Write summary results to output.txt
 ***********************************************************************/

#include "main.h"

int main(int argc, char **argv) 
{
    FILE       *fptr_input  = NULL;
    FILE       *fptr_output = NULL;
    char        line_buffer[4096];
    TokenStats  token_stats = {0};
    LexerState  ls;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_file.c>\n", argv[0]);
        return 1;
    }

    /* check each file independently so we don't leak fptr_input
     * if fptr_output fails to open.                                   */
    fptr_input = fopen(argv[1], "r");
    if (fptr_input == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    fptr_output = fopen("output.txt", "w");
    if (fptr_output == NULL)
    {
        perror("Error opening output file");
        fclose(fptr_input);
        return 1;
    }

    /* initialise lexer state (allocates internal buffer)    */
    lexer_state_init(&ls);

    while (fgets(line_buffer, sizeof(line_buffer), fptr_input))
    {
        line_buffer[strcspn(line_buffer, "\n")] = '\0';

        tokenize_line(line_buffer, fptr_output, &token_stats, &ls);
    }

    /* release the heap buffer inside LexerState             */
    lexer_state_free(&ls);

    fprintf(fptr_output, "\nSummary:\n");
    fprintf(fptr_output, "Keywords:                %d\n", token_stats.keyword);
    fprintf(fptr_output, "Identifiers:             %d\n", token_stats.identifiers);
    fprintf(fptr_output, "Operators:               %d\n", token_stats.operators);
    fprintf(fptr_output, "Integers:                %d\n", token_stats.integers);
    fprintf(fptr_output, "Real Numbers:            %d\n", token_stats.real_numbers);
    fprintf(fptr_output, "String Literals:         %d\n", token_stats.string_literals);
    fprintf(fptr_output, "Delimiters:              %d\n", token_stats.delimiters);
    fprintf(fptr_output, "Preprocessor Directives: %d\n", token_stats.preprocessor);
    fprintf(fptr_output, "Hex/Octal Numbers:       %d\n", token_stats.hex_octal);
    fprintf(fptr_output, "Character Literals:      %d\n", token_stats.character_literals);
    fprintf(fptr_output, "Single Line Comments:    %d\n", token_stats.single_line);
    fprintf(fptr_output, "Multi Line Comments:     %d\n", token_stats.multi_line);

    fclose(fptr_input);
    fclose(fptr_output);
    return 0;
}
