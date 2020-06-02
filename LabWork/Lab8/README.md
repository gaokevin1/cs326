# Lab 8: Signals

You are given a basic signal handling program. Extend it to:
* Reset the counter to 0 every 15 seconds using SIGALRM. Make sure this works every 15 seconds, not just once.
* Add 5 to the counter when the user presses Ctrl+\ (SIGQUIT)
* Add 10 to the counter when SIGUSR1 is received
* Exit gracefully (print the final count and "Goodbye!") when SIGINT is received (already provided)

## Sending Signals

You can send a signal to a process with the `kill` command:

```bash
kill -s SIGQUIT <pid>
```

## Grading (1 minute)

Demo this lab in class to the instructor or TAs to receive credit.
