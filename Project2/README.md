# Project 2

Bash is a shell emulator made in C.
_________________
### How to Build The Program:
To run the program, download the project here from github and in the directory where it is stored, run:
make
./bash
_________________

### Default Functions Included in Shell:
* `cd` Go to a particular directory, or to home when nothing is specified after `cd`
* `jobs` This will display all of the currently running jobs on the system
* `setenv` Sets the enviroment variables for the shell
* `#` To treat command as a comment, and to ignore execution
* History Commands:
* `history` Prints a list of the last 100 commands given to shell by user
* `!prefix` Executes the command from the command history that starts with the prefix given
* `!number` Executes the command from the command history that corresponds to the number given
* `!!` Will run the previous command again. 
* `exit` Quits the program and exits the shell.
_________________

### Pipes & Redirection
This shell can handle piping of commands.
For example if you want to grep the output of `ls`, you can run `ls -a | grep "file*`.

To redirect the output of a particular given command you can use the `>`. 
For example, the command `ls -a > file.txt` would save the output of `ls` into the file `file.txt`.
_________________

### Scripting Mode
Use scripting mode with the standard `<` character.

An example of an executable command would be this: `./bash < commands.txt`
_________________

### Background Jobs
To make a command run as a background job you can use the `&`.
For example, if you type `sleep 100 &` then the command will will run in the background. You can see a list of running background jobs by typing the command `jobs`.
_________________

For more information regarding the program and it's functionality visit:
https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-2.html
