#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <ucontext.h>

/* Preprocessor Directives */
#define NUM_PROCESSES 100
#define STACK_SIZE 8192

/* Process lifecycle phases */
enum process_state {
    CREATED,
    WAITING,
    RUNNING,
    TERMINATED,
};

/**
 * Encapsulates process metadata:
 * - process ID, name
 * - current execution state
 * - memory for the runtime stack
 * - user-level context information
 * - timing information
 */
struct process_ctl_block {
    unsigned int pid;
    char name[128];
    enum process_state state;

    /* Process context information */
    char stack[STACK_SIZE];
    ucontext_t context;

    unsigned int creation_quantum; /* The time slice when process is created */
    unsigned int workload; /* How much work this process will do in total */
    unsigned int executed_work; /* How much work has been done */
    unsigned int priority;

    /* Wall clock times: */
    double arrival_time; /* When the process gets put into the run queue */
    double start_time; /* First time the process actually runs */
    double completion_time; /* When the process completed */
};

/* Create an empty process control block we can use to erase PCBs via assignment
 * (some_pcb = empty_pcb). The following will be automatically zeroed out, so we
 * can rely on it for clearing our PCBs.  */
static const struct process_ctl_block empty_pcb = { 0 };

struct scheduler_state {
    /* Number of processes managed by the scheduler */
    unsigned int num_processes;

    /* Current scheduling quantum, used to determine when tasks arrive. */
    unsigned int current_quantum;

    /* Current running process ID */
    unsigned int current_process;

    /* Array of process control blocks for the processes managed by the
     * scheduler.*/
    struct process_ctl_block pcbs[NUM_PROCESSES];
};

/* Function Prototypes */
void context_switch(int);
struct process_ctl_block *current_pcb(void);
double get_time();
void handle_arrivals(void);
void interrupt_handler(void);
void process(void);
void signal_handler(int);

#endif
