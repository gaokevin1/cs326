# Lab 9: Pipes

One of the core types of IPC on Unix system is pipes. You will be implementing pipe redirection in your shell, and of course there's that [awesome video](https://www.cs.usfca.edu/~mmalensek/cs326/schedule/materials/ATTArchives_UNIX_OS.mp4) of the ATT folks explaining it.

In this lab, you'll get more familiar with pipes (which will then make your life a bit easier when you implement pipes in your shell). Your mission is to produce a super-amazing '[l33tsp34k](https://en.wikipedia.org/wiki/Leet)' generator: given a file name, you will convert its text to lowercase and then perform character substitutions to 1337-ify it. While you are already an elite haxxor and could easily write this program in C, you realize that with the power of Unix you can be even lazier and just combine existing utilities.

To explain the purpose of this program better, let's say you have an input file (input.txt):

```
Mechanic: Somebody set up us the bomb.
Operator: Main screen turn on.
CATS: All your base are belong to us.
CATS: You have no chance to survive make your time.
```

You will run:
```
./leetify input.txt
```

And out comes:
```
m3ch4n!c: 50m3b0dy 53t up u5 t3h b0mb.
0p3r4t0r: m4!n 5cr33n turn 0n.
c4t5: 411 y0ur b453 4r3 b310ng t0 u5.
c4t5: y0u h4v3 n0 ch4nc3 t0 5urv!v3 m4k3 y0ur t!m3.
```

If you run:
```
./leetify input.txt output.txt
```
The contents printed above will be written to a file named output.txt instead.

Truly groundbreaking.

We will use three Unix utilities to achieve this:

* `cat` -- read the original input file
* `tr` -- convert the text to lowercase
* `sed` -- find and replace characters

Check the man pages for all three utilities. **NOTE**: you won't write **any** code that actually does text manipulation. All text manipulation will be provided by outside utilities. You are only gluing these programs together using pipes!

One interesting thing about `tr` is that it only reads files from standard input -- you can't pass in the name of a text file like you can with some Unix utilities. To solve this, we are going to use `cat` to read the file... **but** this is actually considered a [useless use of cat](http://porkmail.org/era/unix/award.html) because most shells already support redirection of `stdin` from a file like so:

```
tr '[:upper:]' '[:lower:]' < ./input.txt
```

This makes the standard input stream of `tr` come from the file `input.txt`. Nevertheless, we will use `cat` to do this instead because it will help with designing our shell pipe support. The equivalent command that we will implement is:

```
cat ./input.txt | tr '[:upper:]' '[:lower:]'
```

The output of `cat` will go into a pipe that we create, which is then read by `tr`, output to a second pipe, read by `sed`, and finally printed to its output stream (either stdout or an output file):

```
cat -> pipe -> tr -> pipe -> sed -> (output file descriptor)
```

To get started, you should refer to the **pipe.c** example -- it shows you how to use `pipe()` and `dup2()`. The **io-redir.c** and **execlp.c** examples may help too; however, you should use `execvp` to execute the commands so that you can easily port this code to your shell. The command line equivalent of this program is:

```
tr '[:upper:]' '[:lower:]' < ./input.txt \
    | sed 's|the|teh|g; s|a|4|g; s|e|3|g; s|i|!|g; s|l|1|g; s|o|0|g; s|s|5|g;' \
    > output.txt
```

So the programs and command line arguments you will execute with `execvp` are:

* `cat 'filename.txt'`
* `tr '[:upper:]' '[:lower:]'`
* `sed 's|the|teh|g; s|a|4|g; s|e|3|g; s|i|!|g; s|l|1|g; s|o|0|g; s|s|5|g;'`

