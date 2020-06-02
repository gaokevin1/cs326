#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#include "debug.h"
#include "history.h"
#include "timer.h"
#include "tokenizer.h"

struct command_line;
// Command_Line Struct
struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};

// Function Prototypes
void print_prompt(void);
void get_info(void);
void execute_pipeline(struct command_line *cmds);
void exec(char *tokens[5000], bool isBackground, char* command_char);
char *tokenizer(char *tokens[5000], char *string);
void sigchld_handler(int signo);
void sigint_handler(int signo);

// Global Data Members
int num_cmds = 0;
int count = 0;
int num_tokens = 0;
int num_jobs[10] = {};
char bg_cmds[10][5000];
char *hostname;
char *username;
char *nameDir;

/**
 * Method prints the command prompt with different information, including:
 * line #, hostname, etc.
 * @param void
 */
void print_prompt(void) {
    // Get Current Working Directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
    }

    char *newDir = NULL;
    char* cwd_ptr = cwd;
    // If the directory names match
    if (strncmp(nameDir, cwd_ptr, strlen(nameDir)) == 0) {
        int length = (int) strlen(nameDir);

        // Cut off the /home/ to create the correct string to display
        cwd_ptr += length;

        char *temp_ptr = malloc(strlen(cwd_ptr) + strlen("~") + 1);
        strcpy(temp_ptr, "~");
        strcat(temp_ptr, cwd_ptr);

        newDir = temp_ptr;
    } else {
        newDir = cwd_ptr;
    }

    printf("--[%d|%s@%s:%s]--$ ", num_cmds, username, hostname, newDir);
    free(newDir);
    fflush(stdout);
}

/**
 * Main Method.
 */
int main(void) {
    // Signal Handlers
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);

    // Get username and hostname info
    get_info();

    /* Loop forever, prompting the user for commands */
    while (true) {
        char* line = NULL;
        size_t line_size = 0;
        char *command_char = NULL;

        if (isatty(STDIN_FILENO)) {
            print_prompt();
        }

        // Check return value of line
        ssize_t size = getline(&line, &line_size, stdin);
        
        /* Free memory if getLine returns -1 */
        if (size == -1) {
            free(command_char);
            free(line);
            break;
        }

        // Copy the current command from the command line for storage in history array later
        command_char = (char*) malloc(line_size); 
        strcpy(command_char, line);

        bool isEmpty = true;
        bool isBackground = false;

        char *tokens[5000];
        tokenizer(tokens, line);

        /* If token array is not empty */
        if (tokens[0] != 0) { 
            isEmpty = false;

            /* Check to make sure that last token is not & */
            if (strcmp(tokens[num_tokens - 1], "&") == 0) {
                // For background jobs
                isBackground = true;
                tokens[num_tokens - 1] = NULL;
            }
        }

        if (!isEmpty) {
            num_cmds++;
            // Boolean to check in command is a built-in command
            bool isBIn = false;

            // Data Members
            char *hisStr;
            char *hisTemp;

            /** If the first token starts with '!' then replace the 
             *  token with the command number found from history.
             */
            if (tokens[0][0] == '!') {
                /* !! command */
                if (tokens[0][1] == '!') {
                    // Get string from history
                    hisStr = get_cmd(num_cmds - 2);
                    hisTemp = (char *)malloc(sizeof(hisStr));
                    strcpy(hisTemp, hisStr);

                    if (hisStr != (char*) NULL) {
                        tokenizer(tokens, hisStr);
                    }
                } else {
                    /* !number command */
                    if (isdigit(tokens[0][1])) {
                        char *temp = tokens[0];
                        temp += 1;
                        int number = atoi(temp);

                        // Get string from history
                        hisStr = get_cmd(number);
                        hisTemp = (char*) malloc(sizeof(hisStr));

                        if (hisStr != (char*) NULL) {
                            strcpy(hisTemp, hisStr);
                            tokenizer(tokens, hisStr);
                        }
                    /* !prefix command */
                    } else {
                        char *temp = tokens[0];
                        temp += 1;

                        hisStr = find_cmd(temp);
                        hisTemp = (char*) malloc(sizeof(hisStr));

                        if (hisStr != (char*) NULL) {
                            strcpy(hisTemp, hisStr);
                            tokenizer(tokens, hisStr);
                        }
                    }
                }
                // Add to command history
                add_history(hisTemp);
            } else {
                // Add to command history
                add_history(command_char);
            }

            /* If command is "exit" */
            if (strcmp(tokens[0], "exit") == 0) {
                printf("\n");
                free(hostname);
                free(nameDir);
                exit(0);
            }

            /* If command is "history" */
            if (strcmp(tokens[0], "history") == 0) {
                isBIn = true;
                print_history();
            }

            /* If command is "cd" */
            if (strcmp(tokens[0], "cd") == 0) {
                isBIn = true;
                /* If no directory provided, cd to home directory */
                if (tokens[1] == NULL) {
                    chdir(nameDir);
                } else {
                    int err = chdir(tokens[1]);
                    if (err == -1) {
                        perror("Directory Not Found\n");
                    }
                }
            }

            /* If command is "num_jobs" */
            if (strcmp(tokens[0], "num_jobs") == 0) {
                isBIn = true;

                int i;
                if (count == 0) {
                    printf("No Background Programs Running\n");
                } else {
                    for (i = 0; i < count; i++) {
                        printf("[%d] %s", i, bg_cmds[i]);
                    }
                }
            }

            /* If command is "setenv" */
            if (strcmp(tokens[0], "setenv") == 0) {
                char var_name[256] = {};
                char var_val[256] = {};

                if (tokens[1] != NULL && tokens[2] != NULL) {
                    strcpy(var_name, tokens[1]);
                    strcpy(var_val, tokens[2]);

                    if (setenv(var_name, var_val, 1)) {
                        fprintf(stderr, "Variable %s not set. (setenv() error)\n", var_name);
                    }
                } else {
                    fprintf(stderr, "Usage Error: setenv 'varname' 'value' ");
                }
            }

            /* If "" */
            if (tokens[0] != 0 && tokens[1] != 0 && tokens[1][0] == '"') {
                tokens[1] = tokens[1] + 1;
                char temp[500];

                strcpy(temp, tokens[1]);
                int length = strlen(temp);

                temp[length - 1] = 0;
                strcpy(tokens[1], temp);
            }

            /* If '\' */
            if (tokens[0] != 0 && tokens[1] != 0 && tokens[1][0] == '\'') {
                tokens[1] = tokens[1] + 1;
                char temp[500];

                strcpy(temp, tokens[1]);
                int length = strlen(temp);

                temp[length - 1] = 0;
                strcpy(tokens[1], temp);
            }

            /* If not a built-in command */
            if (!isBIn) {
                exec(tokens, isBackground, command_char);
            }
        }
        // Free memory used in loop
        free(line);
        free(command_char);
    }
    // Free memory 
    free(hostname);
    free(nameDir);
    return 0;
}

/**
 * Takes all the elements from the tokenized array and puts them in a
 * struct to be used for the pipeline execution.
 * @param tokens array of tokens
 * @param isBackground boolean to show if command is running in background or not
 * @param command_char char array of the command that is running through the pipes
 */
void exec(char *tokens[5000], bool isBackground, char* command_char) {
    // Data Members
    int index = 0;
    int t_count;
    int last = 0;
    bool hasPipe = false;
    char **temp_tokens;
    char **temp_end_tokens;
    char pipechar[2] = "|";

    struct command_line cmds[5000] = {0};

    /* Check for vars in the tokenized command */
    if (tokens[0] != NULL) {
        if (tokens[0][0] == '$') {
            char *check = getenv(tokens[0] + 1);
            if (check == NULL) {
                perror("No Match");
            } else {
                tokens[1] = check;
                fflush(stdout);
            }
        } else if (tokens[1] != NULL && strncmp(tokens[1], "$", 1) == 0) {
            char *check = getenv(tokens[1] + 1);
            if (check == NULL) {
                perror("No Match");
            } else {
                tokens[1] = check;
            }
        }
    }

    /* Parse the tokenized array for adding into the new struct */
    for (t_count = 0; tokens[t_count] != NULL; t_count++) {
        /* If the pipechar '|' is found */
        if (strncmp(tokens[t_count], pipechar, strlen(pipechar)) == 0) {
            temp_tokens = (char**) malloc(5000);

            int current;
            if (hasPipe == true) {
                current = last + 1;
            } else {
                current = last;
            }

            int pos = 0;
            for (; current <= t_count; current++) {
                temp_tokens[pos] = tokens[current];
                if (strcmp(tokens[current], "|") == 0) {
                    break;
                }
                pos++;
            }
            temp_tokens[pos] = NULL;

            a b | c d | e f > temp.txt

            // Move all the elements from token array into the struct
            cmds[index].tokens = temp_tokens;
            cmds[index].stdout_pipe = true;

            // Increment counter variables
            last = t_count;
            index++;
            // Set new boolean for hasPipe value to true
            hasPipe = true;
        }

        /* If the pipechar '>' is found */
        if (strncmp(tokens[t_count], ">", 1) == 0) { 
            temp_tokens = (char**) malloc(5000);

            int current;
            if (hasPipe == true) {
                current = last + 1;
            } else {
                current = last;
            }

            int pos = 0;
            for (; current <= t_count; current++) {
                temp_tokens[pos] = tokens[current];

                if (strcmp(tokens[current], ">") == 0) {
                    break;
                }
                pos++;
            }
            temp_tokens[pos] = NULL;

            // Move all the elements from token array into the struct
            cmds[index].tokens = temp_tokens;
            cmds[index].stdout_pipe = false;
            // Add after the > "temp.txt" with current + 1
            cmds[index].stdout_file = tokens[current + 1];

            // Increment counter variables
            last = t_count;
            index++;
            hasPipe = true;
        }

        /* Add last tokens from the array into the struct for piping */
        if (tokens[t_count + 1] == NULL) {
            temp_end_tokens = (char**) malloc(5000);

            int current;
            if (hasPipe == true) {
                current = last + 1;
            } else {
                current = last;
            }

            int pos = 0;
            for (; current <= t_count; current++) {
                temp_end_tokens[pos] = tokens[current];
                pos++;
            }
            temp_end_tokens[pos] = NULL;

            // Move all the elements from token array into the struct
            cmds[index].tokens = temp_end_tokens;
            cmds[index].stdout_pipe = false;
        }
    }

    /* Call execute_pipeline() to use pipes with the tokens */
    if (cmds[0].tokens != NULL) {
        int pid = fork();
        if (pid == -1) {
            perror("fork");
            return;
        } else if (pid == 0) {
            execute_pipeline(cmds);
        } else {
            int status;
            // If the job is a background job, when sig handler is triggered
            if (isBackground) {
                num_jobs[count] = pid;
                strcpy(bg_cmds[count], command_char);
                count++;
            } else {
                // Don't want to run if it is background job
                waitpid(pid, &status, 0);
            }
        }
    }
}

/**
 * Method that is used to tokenize input commands
 * @param tokens array that contains all of the tokens
 * @param string the command
 * @return
 */
char *tokenizer(char *tokens[5000], char *str) {
    int i = 0;
    char *next_tok = str;
    char *curr_tok;

    /* Add lines that begin with '# as marker' */
    if (strncmp(str, "#", 1) == 0) {
        num_cmds++;
        add_history(str);
    }

    /* Tokenize all of the commands */
    while (i <= 5000 && (curr_tok = next_token(&next_tok, " \t\r\n")) != NULL && curr_tok[0] != '#') {
        tokens[i++] = curr_tok;
        /* If tokens array no longer has any space */
        if (i == 5000) {
            // Set first token to NULL
            tokens[0] = (char*) NULL;
        }
    }

    num_tokens = i;
    tokens[i] = (char *) NULL;
    return *tokens;
}

/***
 * Pipes inplmentation for the command_line
 * @param cmds A struct of different commands
 */
void execute_pipeline(struct command_line *cmds) {
    /**
     * For a list of flags, see man 2 open:
     *
     * - O_RDWR - open for reading and writing (we could get by with O_WRONLY
     *            instead)
     * - O_CREAT - create file if it does not exist
     * - O_TRUNC - truncate size to 0
     */
    int open_flags = O_RDWR | O_CREAT | O_TRUNC;
    
    /**
     * These are the file permissions we see when doing an `ls -l`: we can
     * restrict access to the file. 0644 is octal notation for allowing the user
     * to read and write the file, while everyone else can only read it.
     */
    int open_perms = 0644;

    /* If last command */
    if (cmds-> stdout_pipe == false) {
        if (cmds-> stdout_file != NULL) {
            int fd = open(cmds-> stdout_file, open_flags, open_perms);
            if (fd == -1) {
                perror("open");
                return;
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                close(fd);
                return;
            }
        }
        
        // Check for errors
        int err = execvp(cmds-> tokens[0], cmds-> tokens);
        if (err == -1) {
            fprintf(stderr, "-bash: %s: command not found\n", cmds-> tokens[0]);
            fclose(stdin);
            return;
        }
        return;
    } else {
        int p[2];
        if (pipe(p) == -1) {
            perror("pipe");
            return;
        }

        int pid = fork();
        if (pid == -1) {
            perror("fork");
            return;
        }
        /* If child */
        if (pid == 0) {
            dup2(p[1], STDOUT_FILENO);
            close(p[0]);
            execvp(cmds-> tokens[0], cmds-> tokens);
        /* If parent */
        } else {
            dup2(p[0], STDIN_FILENO);
            close(p[1]);
            execute_pipeline(cmds + 1);
        }
    }
}

/**
 * Gets the hostname and username and stores the information in globally
 * accessible variables.
 */
void get_info(void) {
    // Fetch Hostname
    hostname = (char*) malloc(sizeof(char) * 128);
    int hostVal = gethostname(hostname, 128);
    /* If hostname is not valid, or cannot be determined */
    if (hostVal == -1) {
        strcpy(hostname, "Unknown");
    }

    // Fetch Username
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    username = pwd-> pw_name;

    // Build directory for usernames and hostnames to be stored
    int size = 1000;
    /* If username is valid */
    if (username != NULL) {
        // Dynamically allocated memory for username and hostname storage
        size = sizeof(username) + sizeof(hostname);
    }

    nameDir = (char*) malloc(size + 9);
    strcpy(nameDir, "/home/");
    strcat(nameDir, username);
}

/**
 * Child handler to look over background jobs and add them to an array.
 * @param signo
 */
void sigchld_handler(int signo) {
    int pid = waitpid(-1, NULL, WNOHANG);
    if (pid == -1) {
        perror("background");
    }

    /* If on the last job */
    if (pid == num_jobs[9]) {
        // Reduce count by 1
        count--; 
        strcpy(bg_cmds[9], "");
        num_jobs[9] = 0;
    } else {
        int i = 0;
        while (i < 9 && num_jobs[i] != pid) {
            i++;
        }

        /* Break out of handler */
        if (i == 9) {
            return;
        }

        while (i < count) {
            num_jobs[i] = num_jobs[i + 1];
            strcpy(bg_cmds[i], bg_cmds[i + 1]);
            i++;
        }
        // Reduce count by 1
        count--;
    }
}

/**
 * Interrupt handler to catch program end keystrokes.
 * @param signo
 */
void sigint_handler(int signo) {
    LOG("SIGINT HIT! Signo: %d\n", signo);
}