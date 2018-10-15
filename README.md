# tinyShell
## Description
C program that utilizes different ways to pipe and create child processes to compare functionality and performance in a shell.
## Instructions 
Use makefile to run a given version of the tiny shell and then run with `./tshell`:
* system() version: `make`
* fork() version: `make fork`
* vfork() version: `make vfork`
* clone() version : `make clone`
* pipe version : 
    1. `make pipe`
    2. `./tshell pipepathname w|r` where w means write end, r means read end.
