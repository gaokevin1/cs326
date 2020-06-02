#include "tokenizer.h"
#include <string.h>
#include <stdio.h>

/**
 * Retrieves the next token from a string for the tokenizer
 *
 * @param str_ptr token string that will be analyzed and manipulated
 * @param delim delimiter characters that will be used
 * 
 * @return char pointer to the next token
 */
char *next_token(char **str_ptr, const char *delim)
{
    char quotes[] = "\'\"";
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_st = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_st, delim);

    size_t quote_st = strcspn(*str_ptr, quotes);
    size_t offset = quote_st + 1;
    size_t quote_end;

    if (quote_st < tok_end) {
        quotes[0] = *(*str_ptr + quote_st);
        quotes[1] = '\0';

        quote_end = strcspn(*str_ptr + offset, quotes) + offset;
        tok_end = strcspn(*str_ptr + quote_end, delim) + quote_end - tok_st;
    }

    /* If token zero has been reached */
    if (tok_end <= 0) {
        // Finish tokenizer
        *str_ptr = NULL;
        return NULL;
    }

    // Save the current pointer that the tokenizer is on
    char *current_ptr = *str_ptr + tok_st;
    // Point pointer to the end of the current token
    *str_ptr += tok_st + tok_end;

    /* If at the end of the string */
    if (**str_ptr == '\0') {
        *str_ptr = NULL;
    } else {
        **str_ptr = '\0';

        // Shift pointer to newly placed NUL character at end
        (*str_ptr)++;
    }

    return current_ptr;
}