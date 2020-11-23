# Small Shell
This project emulated a Linux shell in C, similar to bash but without some more advanced features. The shell allows redirection of standard input and standard output and supports both foreground and background processes (controllable by the command line and by receiving signals). The shell supports three built in commands: exit, cd, and status. It also supports comments, which are lines beginning with the # character.

# Usage:

to run the makefile, just type:
make 

or to compile and run,

type:
gcc -o smallsh source.c smallsh.c

then type:
smallsh or
./smallsh
