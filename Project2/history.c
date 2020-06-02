#include "history.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"

// Global Data Members
char history[100][100000];
int minimum = 0;
int cmd_count = 0;
int tail = 0;
int max = 100;

/***
 * Print all of the commands in the history array.
 */
void print_history() {
    int i = tail + 1;
    int j = cmd_count;

    int max_length = tail + max + 1;

    for (; i < max_length; i++, j++) {
        if (strcmp(history[i % max], "") != 0) {
            printf("%d %s", j - max, history[i % max]);
        }
    }
}

/***
 * Add the latest command to the array of history commands.
 * @param character array of a particular command
 */
void add_history(char* command) {
    tail = cmd_count % max;

    strcpy(history[cmd_count % max], command);
    cmd_count++;
    
    if (cmd_count > max) {
        minimum++;
    }
}

/***
 * Returns the command with given command number from history
 * @param number the number of the command desired
 * @return the desired command.
 */
char* get_cmd(int num) {
    /* If desired command is within count range */
    if (num >= minimum && num <= cmd_count) {
        char *temp_ptr = (char*) malloc(sizeof(history[num % max]) + 1);
        strcpy(temp_ptr, history[num % max]);
        return temp_ptr;
    } else {
        // Return NULL when nothing is found
        return (char*) NULL;
    }
}

/***
 * Find a command given a char array prefix from the command history array.
 * @param cmd the prefix of the command that is desired
 * @return the whole command that was found in history, or NULL
 */
char* find_cmd(char* cmd) {
    // Initialize i variable for iterating through array with while loop
    int i = cmd_count - 1;

    while (i >= minimum) {
        if (get_cmd(i) != NULL) {
            if (strncmp(get_cmd(i), cmd, strlen(cmd)) == 0) { 
                char *temp = (char*) malloc(sizeof(get_cmd(i)) + 1);
                strcpy(temp, get_cmd(i));
                return temp;
            }
        }
        i--;
    }
    // Return NULL when nothing is found
    return (char*) NULL;
}