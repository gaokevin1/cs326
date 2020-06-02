#ifndef _HISTORY_H_
#define _HISTORY_H_

struct history_value {
    unsigned long cmd_id;
    double run_time;
};

void print_history();
void add_history(char* command);
char *find_cmd(char* cmd);
char *get_cmd(int number);

#endif
