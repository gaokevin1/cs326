#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};

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

    // If last command
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
        execvp(cmds-> tokens[0], cmds-> tokens);
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
        // If child
        if (pid == 0) {
            dup2(p[1], STDOUT_FILENO);
            close(p[0]);
            execvp(cmds-> tokens[0], cmds-> tokens);
        // If parent
        } else {
            dup2(p[0], STDIN_FILENO);
            close(p[1]);
            execute_pipeline(cmds + 1);
        }
    }
    /**
     * TODO: design a recursive algorithm that sets up a pipeline piece by
     * piece. Imagine you have three commands in a pipeline:
     *
     *  - cat
     *  - tr
     *  - sed
     *
     * This will result in three calls to execute_pipeline, one for each
     * command. Use 'stdout_pipe' to determine when you've reached the last
     * command, and 'stdout_file' to decide whether the final result gets
     * written to a file or the terminal.
     */

    /**
     * (1)
     * Here's our base case for our recursive execute_pipeline function. If
     * there are no more commands to run (stdout_pipe is false), then we just
     * need to execvp the last command. If redirection is enabled (stdout_file
     * is not NULL), then we also send stdout to the specified file.
     *
     * if there are no more commands:
     *     if stdout_file is not null:
     *         open stdout_file
     *         dup2 stdout to the file
     *
     *     execvp the command
     *     return
     */
    
    /**
     * (2)
     * If we aren't at the last command, then we need to set up a pipe for the
     * current command's output to go into. For example, let's say our command
     * is `cat file.txt`. We will create a pipe and have the stdout of the
     * command directed to the pipe. Before doing the recursive call, we'll set
     * up the stdin of the next process to come from the pipe, and
     * execute_pipeline will run whatever command comes next (for instance,
     * `tr '[:upper:]' '[:lower:]'`).
     *
     * create a new pipe
     * fork a new process
     * if pid is the child:
     *     dup2 stdout to pipe[1]
     *     close pipe[0]
     *     execvp the command
     * if pid is the parent:
     *     dup2 stdin to pipe[0]
     *     close pipe[1]
     *     call execute_pipeline on the next command
     */

}

int main(int argc, char *argv[])
{
    char *input_file = NULL;
    char *output_file = NULL;

    if (argc < 2 || argc > 3) {
        printf("Usage: %s file-to-leetify [output-file]\n", argv[0]);
        return 1;
    }

    input_file = argv[1];

    if (argc == 3) {
        output_file = argv[2];
    }

    char *command1[] = { "cat", input_file, (char *) NULL };
    char *command2[] = { "tr", "[:upper:]", "[:lower:]", (char *) NULL };
    char *command3[] = { "sed", "s|the|teh|g; s|a|4|g; s|e|3|g; s|i|!|g; s|l|1|g; s|o|0|g; s|s|5|g;", (char *) NULL };

    struct command_line cmds[3] = { 0 };
    cmds[0].tokens = command1;
    cmds[0].stdout_pipe = true;
    cmds[0].stdout_file = NULL;

    cmds[1].tokens = command2;
    cmds[1].stdout_pipe = true;
    cmds[1].stdout_file = NULL;

    cmds[2].tokens = command3;
    cmds[2].stdout_pipe = false;
    cmds[2].stdout_file = output_file;

    int pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    // If child
    } else if (pid == 0) {
        execute_pipeline(cmds);
    } else {
        int status;
        pid_t pidT = wait(&status);

        if (pidT == -1) {
            perror("wait");
            return -1;
        }
    }
    return 0;
}
