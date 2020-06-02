# Lab 6: Intercepting System Calls

Alright, so we have a somewhat reasonable idea of what system calls are: 
privileged functions that are processed in kernel space. Let's imagine that
we want to add our own system call to Linux. The process involves adding a
new entry to the [system call table](./syscall_32.tbl), which is basically
a big list of **function pointers**. This list maps system calls (represented
internally as integers) to their corresponding function implementations. If
we wanted to add a new system call, we'd simply update the table and provide
an implementation. In the past, system calls could be added or modified at 
run time as loadable **kernel modules**, but newer versions of the Linux
kernel protect the system call table (probably a good idea from a security
standpoint).

In this lab, we will use dynamic loading to intercept user space programs'
system calls and inject code of our own. This allows us to do many interesting
things. You might be wondering how we can intercept system calls -- after all,
they are handled in kernel space. While we could certainly modify the Linux
source code and recompile the kernel, we have another option: `LD_PRELOAD`.

## LD_PRELOAD

The `LD_PRELOAD` environment variable specifies libraries that should be loaded
before a program starts. We also know that most 'system calls'  we use in
our C programs are actually C library functions that call the 'real'
system calls, so we can exploit this fact to intercept the system calls before
they even happen! Something like:

```
┌─────────────┐             ┌─────────────┐             ┌─────────────┐
│ Application │────────────▶│  C Library  │────────────▶│   Kernel    │
└─────────────┘             └─────────────┘             └─────────────┘

                             -- Becomes --

┌─────────────┐             ┌─────────────┐             ┌─────────────┐
│ Application │──┐       ┌─▶│  C Library  │────────────▶│   Kernel    │
└─────────────┘  │       │  └─────────────┘             └─────────────┘
           ┌─────┘       └─────┐
           │  ┌─────────────┐  │
           └─▶│  Our Code   │──┘
              └─────────────┘
```

To do this, we will build a shared object (.so) file, add it to the `LD_PRELOAD`
environment variable, and then intercept any system calls we are interested in.
In fact, we can intercept any call (we could inject code before `printf`, for
instance, if we wanted to).

## Part 1: File Logger

For our first trick, let's build a loadable library that logs each file opened
by the user. The extra-studious folks out there are probably already thinking
about intercepting calls to `open()` -- and that's just what we'll do!

`open-log.c` logs each call to `open()`. Pay particular attention to the `dlsym()`
function: it finds the original address of `open()` and saves it so we can still
call the original version of the function once we're done messing with the
implementation of `open()`. We also pass the `RTLD_NEXT` option. From the man
pages:

```
       RTLD_NEXT
              Find the next occurrence of the desired symbol in the search order after the current
              object.   This  allows  one to provide a wrapper around a function in another shared
              object, so that, for example, the definition of a function  in  a  preloaded  shared
              object (see LD_PRELOAD in ld.so(8)) can find and invoke the "real" function provided
              in another shared object (or for that matter, the "next" definition of the  function
              in cases where there are multiple layers of preloading).
```

The weird C incantation near the top of the file creates a function pointer to the "original" `open()`:

```
int (*orig_open)(const char * pathname, int flags) = NULL;
```

It can be called like so:

```
(*orig_open)(pathname, flags);
```

Use the provided Makefile to build open-log.so. Once built, we'll add it to our `LD_PRELOAD`
and try it on a **single** command:

```
$ LD_PRELOAD=$(pwd)/open-log.so uptime
open-log.c:27:initialize(): Original open() location: 0x7f7de12d5470
open-log.c:28:initialize(): New open() location: 0x7f7de15ea1b9
open-log.c:33:open(): Opening file: /proc/uptime
open-log.c:33:open(): Opening file: /proc/loadavg
 23:12:21 up 1 day, 13:01,  9 users,  load average: 0.00, 0.00, 0.00
```

You can also make this change more permanent by exporting `LD_PRELOAD`. Then all following commands
will load your shared library:

```
export LD_PRELOAD=$(pwd)/open-log.so
```

To make it permanent across shell sessions, add the export to your .bashrc / .bash_profile.


## Part 2: Rick Ropen()

Using the code provided for Part 1, modify it so that:

1. If the user tries to open any files that end in `.java`, report that they do not exist. Hint: think about what `open()` should return; you'll also need to set the `errno` (see `man errno.h`)

2. If the user opens a `.txt` file, you should instead redirect the request to open `roll.txt` provided in this repo. In other words, if you have a file named `hello.txt` that contains 'Hello world!', when you open it you will see the contents of `roll.txt` instead. [Some Context](https://en.wikipedia.org/wiki/Rickrolling).


## Part 3: Hidden Directory, Crouching File Descriptor

Here we want to create a directory that nobody else can see, even if they perform an `ls -a`. We can do this by intercepting calls to `readdir`; if the call is about to return the directory in question, we'll just skip over and return the next file/directory instead.

Use `/secret` as your hidden directory. When doing an `ls -a /`, the directory should not appear (assuming you have your `hidden.so` library loaded). Unfortunately, there is no way to know the full path of the files/directories returned by `readdir`, so you will also have to intercept `opendir` calls. When the root directory is opened, store the resulting file descriptor so you can check `dirp->fd` later to verify the FDs are the ones you're interested in. In other words, we want to make sure that only `/secret` is hidden, **not** every directory named `secret`.

You should handle `closedir` as well to remove FDs from your array when they are closed.


## Part 4: Ghost in the Machine

You now know how to hide directories, and in Project 1 you've gotten a little more familiar with `/proc`. This knowledge should be enough to completely hide our processes from view; if they don't appear in `/proc`, then utilities such as `top` or `ps` won't be able to see them any more. Create a shared library that effectively removes all processes from `/proc` that are owned by your own user ID (run `id` to find out its numeric value).
