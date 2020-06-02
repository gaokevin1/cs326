#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Preprocessor Directives */
#ifndef DEBUG
#define DEBUG 1
#endif
/**
 * Logging functionality. Set DEBUG to 1 to enable logging, 0 to disable.
 */
#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)


/* Function prototypes */
void print_usage(char *argv[]);

char *next_token(char **str_ptr, const char *delim);
char *getNextTok(char *buff, char *sep, char *str, int tokenAfter);
char *getUser(uid_t uid);
void printUptime(int uptime);
void printProgress(int count, int progressRange);
void printUptime(int t);

bool is_dir(const char* path);

struct process_info *getProcessInfo();

/* This struct is a collection of booleans that controls whether or not the
 * various sections of the output are enabled. */
struct view_opts {
    bool hardware;
    bool system;
    bool task_list;
    bool task_summary;
};

/* This struct is a collection of information that pertains to each
 * of the various processes found running on the machine. */
struct process_info {
    int pid;
    char user[100];
    char tasks[20];
    char state[20];
};

void print_usage(char *argv[])
{
    printf("Usage: %s [-ahlrst] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -a              Display all (equivalent to -lrst, default)\n"
"    * -h              Help/usage information\n"
"    * -l              Task List\n"
"    * -p procfs_dir   Change the expected procfs mount point (default: /proc)\n"
"    * -r              Hardware Information\n"
"    * -s              System Information\n"
"    * -t              Task Information\n");
    printf("\n");
}


int main(int argc, char *argv[])
{
    // Default location of the proc file system
    char *procfs_loc = "/proc";

    // Set to true if we are using a non-default proc location
    bool alt_proc = false;

    struct view_opts all_on = { true, true, true, true };
    struct view_opts options = { false, false, false, false };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahlp:rst")) != -1) 
    {
        switch (c) 
        {
            case 'a':
                options = all_on;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 'l':
                options.task_list = true;
                break;
            case 'p':
                procfs_loc = optarg;
                alt_proc = true;
                break;
            case 'r':
                options.hardware = true;
                break;
            case 's':
                options.system = true;
                break;
            case 't':
                options.task_summary = true;
                break;
            case '?':
                if (optopt == 'p') 
                {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } 
                else if (isprint(optopt)) 
                {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } 
                else 
                {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }

                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (alt_proc == true) 
    {
        LOG("Using alternative proc directory: %s\n", procfs_loc);

        chdir(procfs_loc);

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }
        /* Remove two arguments from the count: one for -p, one for the
         * directory passed in: */
        argc = argc - 2;
    } else {
        chdir("/proc");

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }
    }

    if (argc <= 1) {
        // No args (or -p only). Enable all options:
        options = all_on;
    }

    LOG("Options selected: %s%s%s%s\n",
            options.hardware ? "hardware " : "",
            options.system ? "system " : "",
            options.task_list ? "task_list " : "",
            options.task_summary ? "task_summary" : "");

    // Buffer array to use throughout main
    char buf[100000];
    // Range of progress bar
    int progressRange = 20;

    /*-----------------------System Information-----------------------*/

    if (options.system) {
        // Data Members
        char host[100000] = {};
        char kernel[100000] = {};
        char uptime[100000] = {};
        
        /* Hostname */
        int file = open("sys/kernel/hostname", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 1000);
            char *tok = getNextTok(buf, " \n", "NONE", 0);
            // Put everything in a string
            strcpy(host, tok);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        /* Linux Kernel Version */
        file = open("version", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 1000);
            char *tok = getNextTok(buf, " \n", "version", 0);
            // Put everything in a string
            strcpy(kernel, tok);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        // Uptime
        file = open("uptime", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 1000);
            char *tok = getNextTok(buf, " \n", "NONE", 0);
            // Put everything in a string
            strcpy(uptime, tok);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        // Printing Information
        printf("System Information\n------------------\n");
        printf("Hostname: %s\n", host);
        printf("Kernel Version: %s\n", kernel);

        printUptime(atoi(uptime));
    }

    /*-----------------------Hardware Information-----------------------*/

    if (options.hardware) {
        // Data Members
        char cpuname[100000] = {};
        char cores[1000] = {};
        char loadavg[100000] = {};
        char meminfo[100000] = {};
        char cpuinfo[1000] = {};

        /* CPU Model */
        int file = open("cpuinfo", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 1000);
            char *tok = getNextTok(buf, "\t\n", "model name", 0);
            // Put everything in a string
            strcpy(cpuname, tok + 2);
        }
        close(file);

        /* Processing Cores */
        file = open("stat", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            int i = -1;
            read(file, buf, 100000);
            char *next = buf;
            char *curr = buf;

            // Count number of times char 'c' appears for cpu stats
            while (*curr == 'c') {
                curr = next_token(&next, "\n");
                i++;
            }
            
            // If i = 0, then only one core exists
            if (i == 0) {
                strcpy(cores, "1");
            } else {
                // Put everything in a string, --i is for total cpu stats
                sprintf(cores, "%d", --i);
            }
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        /* Load Average */
        file = open("loadavg", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n", "NONE", 0);
            // Put everything in a string
            strcpy(loadavg, tok);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        /* CPU Usage */
        int prevIdle = 0, currIdle = 0;
        int prevTotal = 0, currTotal = 0;

        for (int i = 0; i < 8; i++) {
        	file = open("stat", O_RDONLY);
        	// Make sure file is opened correctly
        	if (file != -1) {
            	read(file, buf, 1000);
            	char *tok = getNextTok(buf, " ", "cpu", i);
            	// Put everything in a string
            	strcpy(cpuinfo, tok);

            	// Add to cpuTimeBoot
            	prevTotal += atoi(cpuinfo);
        	} else {
            	return EXIT_FAILURE;
        	}
        	close(file);
        }

        file = open("stat", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 1000);
            char *tok = getNextTok(buf, " ", "cpu", 3);
            // Put everything in a string
            strcpy(cpuinfo, tok);

            // Add to cpuTimeBoot
            prevIdle = atoi(cpuinfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        // Sleep for 1 second to record second reading of CPU usage
        sleep(1);

        for (int i = 0; i < 8; i++) {
        	file = open("stat", O_RDONLY);
        	// Make sure file is opened correctly
        	if (file != -1) {
            	read(file, buf, 1000);
            	char *tok = getNextTok(buf, " ", "cpu", i);
            	// Put everything in a string
            	strcpy(cpuinfo, tok);

            	// Add to cpuTimeBoot
            	currTotal += atoi(cpuinfo);
        	} else {
            	return EXIT_FAILURE;
        	}
        	close(file);
        }

        file = open("stat", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 1000);
            char *tok = getNextTok(buf, " ", "cpu", 3);
            // Put everything in a string
            strcpy(cpuinfo, tok);

            // Add to cpuTimeBoot
            currIdle = atoi(cpuinfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        // Temporary variables to store difference of measured CPU values
        float diffIdle = currIdle - prevIdle;
        float diffTotal = currTotal - prevTotal;

        // Arithmetic to calculate percentage of CPU usage
        float cpuUsage = (100 * (diffTotal / (diffIdle + 5)) / 10);

        /* Memory Usage */
        int memTotal = 0, memFree = 0, buffers = 0, cached = 0, sCached = 0;

        file = open("meminfo", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "MemTotal:", 0);
            // Put everything in a string
            strcpy(meminfo, tok);

            memTotal = atoi(meminfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);

        file = open("meminfo", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1){
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "MemFree:", 0);
            // Put everything in a string
            strcpy(meminfo, tok);

            memFree = atoi(meminfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);
        
        file = open("meminfo", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1){
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "Buffers:", 0);
            // Put everything in a string
            strcpy(meminfo, tok);
            
            buffers = atoi(meminfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);
        
        file = open("meminfo", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1){
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "Cached:", 0);
            // Put everything in a string
            strcpy(meminfo, tok);
            
            cached = atoi(meminfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);
        
        file = open("meminfo", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1){
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "SwapCached:", 0);
            // Put everything in a string
            strcpy(meminfo, tok);
            
            sCached = atoi(meminfo);
        } else {
            return EXIT_FAILURE;
        }
        close(file);
        
        // Used Memory = Total - (Free + Buffers + Cached + SwapCached)
        float usedMemory = (float) memTotal - (float) (memFree + buffers + cached + sCached);
        float memoryPer = 100 * (usedMemory / memTotal);

        // Printing Information
        printf("\nHardware Information\n--------------------\n");
        printf("CPU Model: %s\n", cpuname);
        printf("Processing Units: %s\n", cores);
        printf("Load Average (1/5/15 min):  %.15s\n", loadavg);
        
        printf("CPU Usage:    ");
        printProgress(cpuUsage / 5, progressRange);
        printf("%.1f%%\n", cpuUsage);

        printf("Memory Usage: ");
        printProgress(memoryPer / 5, progressRange);
        printf("%.1f%% (%.1f GB / 1.0 GB)\n", memoryPer, usedMemory / 1000000);
    }

    /*-----------------------Task Information-----------------------*/

    if (options.task_summary) {
        // Data Members
        char interrupts[100000] = {};
        char contexts[100000] = {};
        char forks[1000];

        /* Interrupts */
        int file = open("stat", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "intr", 0);
            // Put everything in a string
            strcpy(interrupts, tok);
        }
        close(file);

        /* Context Switches */
        file = open("stat", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "ctxt", 0);
            // Put everything in a string
            strcpy(contexts, tok);
        }
        close(file);

        /* Forks */
        file = open("stat", O_RDONLY);
        // Make sure file is opened correctly
        if (file != -1) {
            read(file, buf, 100000);
            char *tok = getNextTok(buf, "\n ", "processes", 0);
            // Put everything in a string
            strcpy(forks, tok);
        }
        close(file);

        /* Tasks */
        int tasks = 0;

        DIR *dir;
        if (alt_proc) {
            dir = opendir(".");
        } else {
            dir = opendir("/proc");
        }
        if (dir == NULL) {
            return EXIT_FAILURE;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            // Make sure it is pointing to a directory
            if ((entry-> d_type == DT_DIR)  && (isdigit(*entry-> d_name) != 0)) {
                tasks++;
            }
        }
        closedir(dir);

        // Printing Information
        printf("\nTask Information \n-------------------\n");
        printf("Tasks Running: %d\n", tasks);
        printf("Since boot: \n");
        printf("    Interrupts: %s\n", interrupts);
        printf("    Context Switches: %s\n", contexts);
        printf("    Forks: %s\n", forks);
    }

    /*-----------------------Task List-----------------------*/

    if (options.task_list) {
        // Data Members
        char buffer[100000] = {};
        char fullDir[100000] = {};
        int pid = 0;
        char state[13] = {};
        char name[200] = {};
        char user[16] = {};
        char tasks[5] = {};
        
        // Printing Information
        printf("%5s | %12s | %25s | %15s | %s \n","PID", "State", "Task Name", "User", "Tasks");
        printf("------+--------------+---------------------------+-----------------+-------\n");

        struct dirent *d_struct;
        
        DIR *dir;
        if (alt_proc){
            dir = opendir(".");
        } else {
            dir = opendir("/proc");
        }
        if (dir == NULL){
            return EXIT_FAILURE;
        }
        
        while ((d_struct = readdir(dir)) != NULL) {
            // Make sure it is pointing to a directory
            if ((d_struct-> d_type == DT_DIR) && (isdigit(*d_struct-> d_name) != 0)) {
                strcpy(fullDir, d_struct-> d_name);
                strcat(fullDir, "/status");
                
                int file = open(fullDir, O_RDONLY);
                // Make sure file is opened correctly
                if (file != -1) {
                    read(file, buffer, 100000);
                    
                    char *tok = getNextTok(buffer, " \n\t", "Threads:", 0);
                    // Put everything in a string
                    strcpy(tasks, tok);
                }
                close(file);
                
                file = open(fullDir, O_RDONLY);
                // Make sure file is opened correctly
                if (file != -1) {
                    read(file, buffer, 100000);
                    char *tok = getNextTok(buffer, " \n\t", "Name:", 0);
                    // Put everything in a string
                    strcpy(name, tok);
                }
                close(file);
                
                file = open(fullDir, O_RDONLY);
                // Make sure file is opened correctly
                if (file != -1) {
                    read(file, buffer, 100000);
                    char *tok = getNextTok(buffer, " \n\t", "State:", 0);
                    // Put everything in a string
                    strcpy(state, tok);
                }
                close(file);
                
                // Read in the process IDs and assign each of them to a process name
                sscanf(d_struct-> d_name, "%d", &pid);

                file = open(fullDir, O_RDONLY);
                // Make sure file is opened correctly
                if (file != -1) {
                    read(file, buffer, 100000);
                    char *tok = getNextTok(buffer, " \n\t", "Uid:", 0);

                    // Put everything in a string
                    strcpy(user, getUser(atoi(tok)));
                }
                close(file);
                
                // Check to see what state the process is in by comparing strings
                if (strcmp(state, "S") == 0) {
                    strcpy(state, "sleeping");
                } else if (strcmp(state, "R") == 0) {
                    strcpy(state, "running" );
                } else if (strcmp(state, "I") == 0) {
                    strcpy(state, "idle");
                } else if (strcmp(state, "Z") == 0) {
                    strcpy(state, "zombie");
                } else if (strcmp(state, "D") == 0) {
                    strcpy(state, "disk sleep");
                } else if (strcmp(state, "T") == 0) {
                    strcpy(state, "tracing stop");
                } else {
                    strcpy(state, "unknown");
                }
                
                // Printing Information
                printf("%5d | %12s | %25.25s | %15.15s | %s \n", pid, state, name, user, tasks);
            }
        }
        // Close directory at the end of function call
        closedir(dir);
    }
    return 0;
}

char *getUser(uid_t uid)
{
    struct passwd *pws;
    pws = getpwuid(uid);
    
    return pws-> pw_name;
}

char *getNextTok(char *buff, char *sep, char *str, int nextTok) {
    char *string = buff;
    char *curr_tok = NULL;
    
    if (strcmp(str, "NONE") == 0) {
        curr_tok = next_token(&string, sep);
        return curr_tok;
    } else {
        int i = 0;
        while (i <= 10000) {
            curr_tok = next_token(&string, sep);

            if (strcmp(curr_tok, str) == 0) {
                break;
            }
            i++;
        }
        
        // While loop to find next values along the same tokenized line
        int j = 0;
        while (j < nextTok) {
            curr_tok = next_token(&string, sep);
            j++;
        }

        curr_tok = next_token(&string, sep);
        return curr_tok;
    }
}

void printUptime(int t) {
    int seconds, minutes, days, hours, years;
    // If you need to include years
    if (t > 86400 * 365) {
        // Time Arithmetic
        years = t / (60 * 60 * 24 * 365);
        t -= years * (60 * 60 * 24 * 365);
        days = (t % (86400 * 30)) / 86400;
        hours = (t % 86400) / 3600;
        minutes = (t % 3600) / 60;
        seconds = t % 60;

        if (days == 0 && hours == 0) {
            printf("Uptime: %d years, %d minutes, %d seconds\n", years, minutes, seconds);
        } else {
            printf("Uptime: %d years, %d days, %d hours, %d minutes, %d seconds\n", years, days, hours, minutes,
                   seconds);
        }
    // If you need to include days
    } else if (t > (3600 * 24)) {
        // Time Arithmetic
        days = (t % (86400*30)) / 86400;
        hours = (t % 86400) / 3600;
        minutes = (t % 3600) / 60;
        seconds = t % 60;
        printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
    // If you need to include hours
    } else if (t > 3600) {
        // Time Arithmetic
        hours = (t % 86400) / 3600;
        minutes = (t % 3600) / 60;
        seconds = t % 60;
        printf("Uptime: %d hours, %d minutes, %d seconds\n", hours, minutes, seconds);
    // If you need to include minutes
    } else if (t > 60) {
        // Time Arithmetic
        minutes = (t % 3600) / 60;
        seconds = t % 60;
        printf("Uptime: %d minutes, %d seconds\n", minutes, seconds);
    } else {
        // Time Arithmetic
        seconds = t % 60;
        printf("Uptime: %d seconds\n", seconds);
    }
}

void printProgress(int count, int progressRange)
{
    printf("[");
    // Fill progress bar with '#'s and spaces
    for (int i = 0; i < progressRange; i++) {
        if (i < count) {
            printf("#");
        } else {
            printf("-");
        }
    }
    printf("] ");
}

char *next_token(char **str_ptr, const char *delim) {
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    // Zero length token. We must be finished.
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    // Take note of the start of the current token. We'll return it later.
    char *current_ptr = *str_ptr + tok_start;

    // Shift pointer forward (to the end of the current token)
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } 
    else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }
    return current_ptr;
}