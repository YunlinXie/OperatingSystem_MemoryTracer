# OperatingSystem_MemoryTracer

One of the main problems in memory management with C is memory leaks. Memory leaks in an Operating System cause serious runtime problems and they can make a system unstable.

Implement a program mem_tracer.c that reads all lines from an input file. However, read all lines into an array of type char ** . 

Initialize the array memory with malloc to an initial size. You should use realloc to expand your array if the initial size turns out not to be big enough for the lines in the input file. Your implementation should work with an input file of up to a few hundred lines.

Store each line in a linked list where a node contains the line as a string (char *) and index number (you probably already did something similar for assignment3). Additionally, implement a recursive function PrintNodes that prints out the content of all nodes in the linked list; printing just the line and index number are sufficient. Call PrintNodes to ensure the linked list has the correct content.

Implement tracing of memory consumption. Use a stack holding the function names and print the memory consumed whenever memory is allocated, reallocated or freed. The tracing should print messages describing the memory management and usage in each function.

The allocation and deallocation for the char ** array and your linked list should make use of your memory tracing in order to print messages describing the memory management and usage.

The parent process should redirect the stdout to a log file named "memtrace.out". You can use dup2 for the purpose of redirecting stdout (fd1) to the memtrace.out file. Your dup2() call will replace the file descriptor STDOUT_FILENO with the opened file for memtrace.out.

Ensure there are no memory leaks. Your code should work on an input file with up to a few hundred lines.
