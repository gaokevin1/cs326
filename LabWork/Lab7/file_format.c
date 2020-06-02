/**
 * Handles loading process definition files. The format is:
 *
 * - One process per line
 * - Each line contains the following fields (separated by commas ','):
 *     - Process Name
 *     - Creation Quantum
 *     - Workload Size
 *     - Priority
 * - # for comments
 *
 * While some sanity checking is performed, the format is probably brittle -- so
 * make sure to follow the format more or less exactly as described.
 */

#include "file_format.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Reads a process specification file and populates the scheduler state strut
 * with the processes.
 */
void read_spec(char *file, struct scheduler_state *sstate)
{
    fprintf(stderr, "Reading process specification: %s\n", file);
    const unsigned int line_sz = 1024;
    FILE *fp = fopen(file, "r");
    char line[1024] = { 0 };
    while (fgets(line, 1024, fp) != NULL) {
        int i;
        int fields = 1;
        for (i = 0; i < line_sz; ++i) {
            /* Remove comments (starting with #) from the line. We replace #
             * with the NUL byte to effectively end the string. We also remove
             * spaces and tab characters. */
            if (line[i] == '#') {
                line[i] = '\0';
            }

            if (line[i] == '\0') {
                /* We're done, whether we found one of our NUL bytes or one that
                 * was already in the string. */
                break;
            }

            /* A basic sanity test: making sure there are 3 commas (for four
             * total fields) in the input string. */
            if (line[i] == ',') {
                fields++;
            }
        }

        if (strlen(line) == 0) {
            continue;
        }

        if (fields != 4) {
            fprintf(stderr,
                    "Ignoring malformed line (%d fields): %s\n", fields, line);
            continue;
        }

        struct process_ctl_block *pcb = &sstate->pcbs[sstate->num_processes];
        *pcb = empty_pcb;

        pcb->pid = sstate->num_processes;
        char *next_tok = line;
        char *tok = next_token(&next_tok, ", \t\n");
        strcpy(pcb->name, tok);
        tok = next_token(&next_tok, ", \t\n");
        pcb->creation_quantum = atoi(tok);
        tok = next_token(&next_tok, ", \t\n");
        pcb->workload = atoi(tok);
        tok = next_token(&next_tok, ", \t\n");
        pcb->priority = atoi(tok);

        fprintf(stderr, "Created process: %s\n", pcb->name);
        sstate->num_processes++;
    }
}


/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

