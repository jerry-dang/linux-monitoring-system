# linux-monitoring-system

## This is README file

The system monitor program consists of the three files: stats_functions.c stats_functions.h and main.c. Using the makefile, I will discuss later on how to assemble and run my program.

Some other useful libraries that I used were string.h and math.h, signal.h, stdbool.h and of course all the libraries that were necessary to get all the system information. Here are a list of libraries that I used for this assignment: #include <stdio.h> #include <stdlib.h> #include <unistd.h> #include <string.h> #include <signal.h> #include <stdbool.h> #include <sys/sysinfo.h> #include <sys/utsname.h> #include <sys/resource.h> #include <utmp.h> #include <math.h>

## How I Solved the Problem

My cpu calculation method was off by a little bit and that required a change that was although small, it took me a long time to figure out a solution for it from my previous implementation. After deciding what to do with my new design to incorporate concurrency to my program, I decided to use two files: stats_functions.c , which was recommended on the assignment handout and a main program called main.c, where the functions would be used to print the information concurrently. After trying to compile the two files, it kept giving me an error saying that multiple of my functions in stats_functions.c were being defined multiple times and I decided to create a .h (stats_functions.h) file (defines all the functions that are in stats_functions.c) for it as well and it got rid of that bug. I suspect that that may have been caused by stats_functions.c not having a main() function. Nonetheless, after that was done I started debugging my code and started experimenting with the piping and handler implementation. Catching the signals was a fairly easy task, just had to figure out the signal code and pcrs really helped with my understanding of pipes, signals and how they worked. I eventually came up with a solution that worked and the program is now delivering the information to the user in a concurrent manner. If errors were to occur while reading or writing to pipes or getting information from structs and libraries, they will be handled by printing an error message to stderr with a clear message of what and where the error occurred.

## Overview of The Functions

Each function is documented in the code file itself but here is a brief summary of each function. Function signature is also in the documented comments in the code.

### In stats_functions.c:

memoryUsage() - Acquires the general process memory usage in kilobytes and returns it

memoryAccess() - Gets the ram memory information and stores it in an array[4] of floats and returns the pointer to that array

cpuCores() - Gets the number of cores on the current machine and returns it

void passMemory() - Write to pipe about memory for the current process

void passRam() - Write to pipe about ram memory for the current process

void passUsers() - Write to pipe about active users for the current process

void passCpu() - Write to pipe about cpu utilization for the current process

void passSystem() - Write to pipe about system information for the current process

### In stats_functions.h:

Same as stats_functions.c - this is just where it is defined.

### In main.c:

*Functions helping with formatting the output (self-explanatory) { void separator() void header() void sequential() void terminate() void continuation() void wrong_input() void clear() }

cpuGraphic() - Provdes a graphic of the cpu usage by scaling the number of bars printed in fromt of the cpu usage value relative to a scale of 0-100

ramGraphic() - Provides a graphic of the ram usage by using the symbols "@" ":" "#" and "o" respective to the negative/positive relative change in memory

float computeCpu() - Finds the difference in values between the previous and current sample cpu usage and returns it

void displayMemory() - Prints memory information

void displayRam() - Prints ram memory information

void displayUsers() - Prints users currently active on the machine

void displayCpu() - Prints current cpu utilization of the machine

void displaySystem() - Prints system information of the machine

void handler() - Handles the signal interceptions from the user. If Ctrl-z is detected, skip to next iteration. If Ctrl-c is detected, the user can either respond 'Y' or 'yes' if they want to terminate the program or 'N' or 'no' if they want the program to continue. If neither of these results are detected, then the input is unrecognized and the program won't continue unless a valid input from the user is detected.

int main() - main program that controls and manages the CLAs and the way information is delivered to the screen in a conccurent fashion.

### How to Run/Use My Program

To use my program, the user can choose between a couple of options for the command line arguments.

User flags: - "--system" : if this feature flag is present as an argument, then the user will ONLY be able to see the system usage information such as the physical/virtual memory usage and cpu information and usage. (Note that this flag should be used at the same time as "--users") - "--users" : if this feature flag is present as an argument, then the user will ONLY be able to see how many users are connected in a given point in time and how many sessions each user is connected to. (Note that this flag should be used at the same time as "--system") - "--graphics" : if this feature flag is present as an argument, then the user will be able to see additional details for the "--system" flag as a more visual approach will be displayed to enhance the depth of the information such as differences between memory usage in the system memory and cpu usage. - "--sequential" : if this feature flag is present as an argument, then the user would be able to see every sample generated printed line by line, instead of seeing all the output for every sample. Useful for output redirection if the output is desired to be stored in a file. - *"--samples=N" : if this feature flag is present as an argument with an N value specified, then the user will be able to see (depending on which previous flags are detected) the information printed concurrently through N samples over a tdelay=T amount of time between each sample displayed. If the N value is not specified, the default value of N is 10. - *"--tdelay=T" : if this feature flag is present as an argument with a T value specified, then the user will be able to control the delay in seconds between N samples printed to the console.

IMPORTANT: this feature flag is also implemented as a positional argument, meaning that if this argument is not  present in the command line arguments given by the user, the user can still specify the number of samples by simply providing a number (e.g. 8) as the second last argument will be detected as the number of samples if and only if the number of tdelay is also provided as just a number as the absolute LAST argument provided.

E.g. `./a.out 8 3` (GOOD - if both arguments are provided, the second last is the sample size and last is the delay in secs)

Another example: E.g. `./a.out 8` (BAD - no tdelay number provided and program will not work as intended).

Another example: E.g. `./a.out --samples=4` (GOOD - if only --samples is provided, then tdelay will just be defaulted to 1 sec)

Another example: E.g. `./a.out 8 --tdelay=2` (BAD - cannot mix and match, either provide both as numbers or use --samples and/or tdelay as argument(s))

This applies to tdelay as well, so either both arguments have to be provided in only numbers are the second last and last argument or --samples and --delay have to be specified for a desired number of samples and delay. If neither are provided, then default values will be assigned.

### Running the Program

To run the program, I would type `gcc main.c stats_functions.c` which will compile the file and create an executable "a.out" in the current directory. This will only work if all the c files and .h (stats_functions.h) file is in the same directory. Then, `./a.out ["--system"] ["--users"] ["--graphics"] ["--sequential"] ["--samples=N"] ["--tdelay=T"]` where the arguments in brackets are technically options, it would be the user's choice for which one they would want to see.

Another way would be to use: `./a.out ["--system"] ["--users"] ["--graphics"] ["--sequential"] [any integer] [any integer]` where the last two arguments are positional arguments for --samples and --tdelay but both have to be provided to work as intended as mentioned above.

A third way to run this program would be to make use of the makefile I created for this assignment (which was required). To run the makefile, make sure that all other files namely, stats_functions.h, stats_functions.c, and main.c are currently in the same directory as the makefile. Then type "make" in the command line and in order to run the program, simply type in the command line: ~$ ./systemMonitoringTool.

The command line arguments work the same way as mentioned in the previous two methods but here is the version for the third method: `./systemMonitoringTool ["--system"] ["--users"] ["--graphics"] ["--sequential"] ["--samples=N"] ["--tdelay=T"]` or `./systemMonitoringTool ["--system"] ["--users"] ["--graphics"] ["--sequential"] [any integer] [any integer]` with all the previous dependencies and conditions as well.

*IMPORTANT: For the cpu utilization, the first sample is simply the absolute cpu usage assuming that there is no previous sample, so the number could be a bit large but I believe that it was a good method since it is quite nice when the program is run with graphics and actually shows the difference between that sample and the next ones.

This is the end of this README file.
