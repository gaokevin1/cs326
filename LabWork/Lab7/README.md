# Lab 7: CPU Scheduling

In this lab, we'll be implementing CPU scheduling algorithms. We could actually design our own Linux kernel modules for this, but the complexity is extreme: CFS (Completely Fair Scheduling) is somewhere around 10,000 lines of code!

Instead, we will implement our own *user-space* threading library. As we know, our processes are given a main thread of execution. We'll divide this single thread up across multiple virtual processes, and decide the order in which they run via our scheduling algorithms.

Your mission for this lab is:
1. Implement the scheduling algorithms below
2. Add a new command line option that allows you to select the scheduler. For example, if you run `./scheduler fifo processes1.txt` then the FIFO algorithm will be used.
3. Execution metrics. After the program finishes running, you'll print some stats about the execution.

## Algorithms

You will implement the following scheduling algorithms. **NOTE**: if there are ties (i.e., two processes are both valid candidates to run), use the index order of `sched_state->pcbs` array.

### Basic
This scheduler gives you an example implementation to base your other algorithms off of. It simply iterates through the list of processes, finds the next one that needs to be run, and then context switches to it.

### FIFO - First In, First Out
Similar to the above, but processes are executed in the order of their arrival (first in, first out).

### SJF - Shortest Job First
The shortest process is run next, based on workload size. If a new process arrives, the currently-running process is **NOT** context switched out. This means that for each interrupt, you will simply re-run the last process until it completes.

### PSJF - Preemptive Shortest Job First 
Similar to SJF, but with preemption. If a task with a smaller overall workload arrives, you will switch to it and run it instead.

### SCTF - Shortest Completion Time First
Building on PSJF, SCTF considers the remaining amount of workload rather than the total workload for the job. For example, if the current job has a workload of 10 but has already executed 8/10 work units, a new task with a workload of 3 arriving will not cause a context switch.

### RR - Round Robin
Each process gets a turn. Every time you receive an interrupt, switch to the next process in the list. Once you hit the end of the list, start back at the beginning.

### Priority
The process with the highest priority is run next. If two processes have the same priority, then switch between them round robin style.

### Insanity
Choose a random number. That PID gets to run (unless it has terminated).


## Algorithm Selector

The second command line argument should specify the algorithm as a case-insensitive string (use the shortened versions above, if applicable -- 'rr' for Round Robin, and so on). Modify the main() function to allow switching the algorithm.

## Metrics

You will print:
* Turnaround times
* Response times
* Average turnaround and response time
* Completion order

...at the end of the program's execution. Here's an example run of the program below:

```
$ ./scheduler fifo processes2.txt
Reading process specification: processes2.txt
Created process: Process_A
Created process: Process_B
Created process: Process_C
Calibrating workload...
42949672 -> 0.18s
85899344 -> 0.23s
171798688 -> 0.46s
92529623 -> 0.34s
Starting execution  -> interrupt (0)
[*] New process arrival: Process_A
[*] New process arrival: Process_B
[*] New process arrival: Process_C
[i] Context switch: PID 0 -> PID 0
[>] PID 0 starting ('Process_A', workload: 15)
Process_A [####                ]  20% -> interrupt (1)
Process_A [#########           ]  46% -> interrupt (2)
Process_A [##############      ]  73% -> interrupt (3)
Process_A [####################] 100% -> terminated -> interrupt (4)
[i] Context switch: PID 0 -> PID 1
[>] PID 1 starting ('Process_B', workload: 5)
Process_B [############        ]  60% -> interrupt (5)
Process_B [####################] 100% -> terminated -> interrupt (6)
[i] Context switch: PID 1 -> PID 2
[>] PID 2 starting ('Process_C', workload: 5)
Process_C [############        ]  60% -> interrupt (7)
Process_C [####################] 100% -> terminated

Execution complete. Summary:
----------------------------
Turnaround Times:
 - Process_A 3.90s
 - Process_B 5.16s
 - Process_C 6.51s
Average turnaround time: 5.19s

Response Times:
 - Process_A 0.00s
 - Process_B 3.90s
 - Process_C 5.16s
Average response time: 3.02s

Completion Order:
 0.) Process_A
 1.) Process_B
 2.) Process_C
```

## Evaluation

What is the best scheduling algorithm for the following process specification files? Provide rationale for your answer.

1. processes1.txt -- (algorithm here), because X, Y, Z...
2. processes2.txt -- (algorithm here), because X, Y, Z...
3. processes3.txt -- (algorithm here), because X, Y, Z...
4. processes4.txt -- (algorithm here), because X, Y, Z...
