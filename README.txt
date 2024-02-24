-GCC compiler
-POSIX Threads

Use the provided Makefile for compilation. Run the following command in the terminal:
-make

Run the program using the following command format:
-./phsp <num_phsp> <min_think> <max_think> <min_dine> <max_dine> <dst> <num>

<num_phsp> is the number of philosophers.
<min_think> and <max_think> specify the range of thinking time in milliseconds.
<min_dine> and <max_dine> specify the range of dining time in milliseconds.
<dst> is the distribution of time(exponential).
<num> is the number of times each philosopher will dine.

Example:
-./phsp 5 500 1000 50 100 exponential 100

phsp.c: Contains the main logic for the philosopher's routines and synchronization mechanisms.
Makefile: Simplifies the compilation process.
