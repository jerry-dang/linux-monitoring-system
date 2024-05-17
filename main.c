#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <utmp.h>

#include "stats_functions.h"

# define BUFFER_SIZE 255
# define FUNC_NUM 4 // separated original display function into 4 separate printing functions


// Print functions to help with formatting
void separator() {
    printf("---------------------------------------\n");
}

void header(int *samples, int *tdelay) {
    printf("Nbr of samples: %d -- every %d secs\n", *samples, *tdelay);
}

void sequential(int currSample) {
    printf(">>> iteration %d\n", currSample);
}

void terminate() {
    printf("Terminate the current program? (Y/N), yes/no\n");
}

void continuation() {
    printf("Continuing executing current program...\n");
}

void wrong_input() {
    printf("Unrecognized character(s). Continue to terminate current program? (Y/N), yes/no\n");
}

void clear() {
    printf("\e[1;1H\e[2J");
}

// SCALING
/*******************
 * Function: cpuGraphic()
 * Parameters: float *usage
 * Returns: void (N/A)
 * 
 * This function provides a visual when sampling the cpu usage of the
 * current machine. More '|' signifies a higher usage percentage whereas
 * less '|' represents a lower usage percentage.
*******************/
void cpuGraphic(float *usage) {
    printf("         ");
    for (int i=0; i<(int)(*usage); i++) {
        printf("|");
    }
    printf(" %.2f\n", *usage);
}


/*******************
 * Function: ramGraphic()
 * Parameters: float *prevMemory, float *currMemory, int *sampleFlag
 * Returns: void (N/A)
 * 
 * This function provides a visual display between consecutive samples
 * of RAM memory usage of the current machine. It appends either '#', 'o', ':', or '@' 
 * depending on the physical/virtual memory difference determined from consecutive sample comparisons. 
 * The amount of ':' or '#' is scaled relative to the difference in physical memory compared to the 
 * previous sample's value (':' for relative negative change followed by '@' and '#' for relative positive 
 * change followed by '*'). The sampleFlag variable is simply a flag to check whether or not we are 
 * printing the first sample since it has nothing to compare. If the difference between samples is 
 * negligible, 'o' followed by the bar means a small positive change whereas '@' followed by the bar 
 * means a small negative change from sample to sample.
*******************/
void ramGraphic(float *prevMemory, float *currMemory, int *sampleFlag) {

    float difference;
    if (*sampleFlag == 0) {
        printf("o 0.00 (%.2f)\n", *currMemory);
    } else {
        difference = *currMemory - *prevMemory;
        if (difference > 0 && difference < 0.01) {
            printf("o 0.00 (%.2f)", *currMemory);
            printf("\n");
        } else if (difference == 0) {
            printf("o 0.00 (%.2f)", *currMemory);
            printf("\n");
        } else if (difference < 0 && difference > -0.01) {
            printf("@ 0.00 (%.2f)", *currMemory);
            printf("\n");
        } else if (difference > 0.01) {
            for (int i; i<abs((int)(difference*100)); i++) {
                printf("#");
            }
            printf("* %.2f (%.2f)", difference, *currMemory);
            printf("\n");
        } else if (difference < -0.01) {
            for (int i; i<abs((int)(difference*100)); i++) {
                printf(":");
            }
            printf("@ %.2f (%.2f)", difference, *currMemory);
            printf("\n");
        }
    }
}


/*******************
 * Function: computeCpu(int *sample, float usage[][2])
 * Parameters: int *sample, float usage[][2]
 * Returns: float
 * 
 * This function computes the difference between the first and second element of the double 
 * array float usage[][2]. These values will be filled as the process will write to the pipe fd 
 * and this function is called when the difference between the current and previous iteration 
 * of the cpu utilization is needed and multiplied by 100 to get the cpu percentage usage.
*******************/
float computeCpu(int *sample, float usage[][2]) {

    float currSample = usage[(*sample)][0];
    float prevSample = usage[(*sample)-1][0];
    float currTotal = usage[(*sample)][1];
    float prevTotal = usage[(*sample)-1][1];

    float difference = ((float)(currSample - prevSample) / (float)(currTotal - prevTotal)) * 100;

    return difference;
}

// need to seperate display function from a1
/*******************
 * Function: displayMemory(int pipefd[FUNC_NUM][2])
 * Parameters: int pipefd[FUNC_NUM][2]
 * Returns: void (N/A)
 * 
 * This function prints the memory usage information by reading from the pipe file 
 * descriptor.
*******************/
void displayMemory(int pipefd[FUNC_NUM][2]) {

    long int processUsage;                  // child process
    long int totalUsage = memoryUsage();    // parent process
    for (int i=0; i<FUNC_NUM; i++) {
        if (read(pipefd[i][0], &processUsage, sizeof(long)) == -1) {
            fprintf(stderr, "Error reading memory usage\n");
        }
        totalUsage = totalUsage + processUsage;
    }
    printf(" Memory usage: %ld kilobytes\n", totalUsage);
    separator();
}


/*******************
 * Function: displayRam(int pipefd, float *physical, float *virtual, int *currSample, int samples, int *graphicsF, 
                int *sequentialF)
 * Parameters: int pipefd, float *physical, float *virtual, int *currSample, int samples, int *graphicsF, 
                int *sequentialF
 * Returns: void (N/A)
 * 
 * This function prints the ram memory information based on the pipe file descriptor and the 
 * user's demands. 
*******************/
void displayRam(int pipefd, float *physical, float *virtual, int *currSample, int samples, int *graphicsF, 
                int *sequentialF) {
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
    Memory stats;
    if (read(pipefd, &stats, sizeof(Memory)) == -1) {
        fprintf(stderr, "Error reading ram memory\n");
    }
    float totalPhys = stats.totalRam;
    float totalVirt = stats.totalVirtRam;
    physical[(*currSample)-1] = stats.usedRam;
    virtual[(*currSample)-1] = stats.usedVirtRam;
    if (*sequentialF == 1) {
        int index = *currSample - 1;
        for (int i=0; i<index; i++) {
            printf("\n");
        }
        if (*graphicsF == 1) {
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", 
            physical[index], totalPhys, virtual[index], totalVirt);
            printf("   |");
            if (*currSample == 1) {
                ramGraphic(0, &physical[index], &index);
            } else if (*currSample > 1) {
                ramGraphic(&physical[index-1], &physical[index], &index);
            }
        } else {
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", 
            physical[index], totalPhys, virtual[index], totalVirt);
        }
    } else {
        for (int i=0; i<*currSample; i++) {
            if (*graphicsF == 1) { // graphicsFlag on
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", 
                physical[i], totalPhys, virtual[i], totalVirt);
                printf("   |");
                if (i == 0) {
                    ramGraphic(0, &physical[i], &i);
                } else if (i > 0) {
                    ramGraphic(&physical[i-1], &physical[i], &i);
                }
            } else { // graphicsFlag off
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", 
                physical[i], totalPhys, virtual[i], totalVirt);
            }
        }
    }   
    for (int j=0; j<samples-(*currSample); j++) {
        printf("\n");
    }
    separator();
}


/*******************
 * Function: displayUsers(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function prints the current active human users by reading from the pipe file 
 * descriptor.
*******************/
void displayUsers(int pipefd) {

    printf("### Sessions/users ###\n");
    struct utmp buffer;
    int bytes;
    // read returns number of bytes read
    while ((bytes = read(pipefd, &buffer, sizeof(struct utmp))) > 0) {
        if (bytes == -1) {
            fprintf(stderr, "Error reading users\n");
        }
        printf("%s       %s (%s) \n", buffer.ut_user, buffer.ut_line, buffer.ut_host);
    }
    separator();
}


/*******************
 * Function: displayCpu(int pipefd, int *currSample, int samples, float *cpuDifference, float cpuPercent[][2], 
                int *graphicsF, int *sequentialF)
 * Parameters: int pipefd, int *currSample, int samples, float *cpuDifference, float cpuPercent[][2], 
                int *graphicsF, int *sequentialF
 * Returns: void (N/A)
 * 
 * This function prints the cpu information depending on the user's demands with feature flags as 
 * parameters. It does so by reading from the pipe file descriptor.
*******************/
void displayCpu(int pipefd, int *currSample, int samples, float *cpuDifference, float cpuPercent[][2], 
                int *graphicsF, int *sequentialF) {
    
    Procutil info;

    if (read(pipefd, &info, sizeof(Procutil)) == -1) {
        fprintf(stderr, "Error reading from pipe (cpu)\n");
    }

    cpuPercent[*currSample][0] = info.utilization;
    cpuPercent[*currSample][1] = info.total;
    cpuDifference[(*currSample)-1] = computeCpu(currSample, cpuPercent);

    printf("Number of cores: %d\n", info.count);
    printf(" total cpu use = %.2f%%\n", cpuDifference[(*currSample)-1]);

    if (*sequentialF == 1) {
        printf(">>> iteration %d\n", *currSample);
        int cpuIndex = *currSample - 1;
        for (int i=0; i<cpuIndex; i++) {
            printf("\n");
        }
        if (*graphicsF == 1) {
            cpuGraphic(&cpuDifference[cpuIndex]);
        } else {
            printf("         %.2f\n", cpuDifference[cpuIndex]);
        }
    } else {
        for (int i=0; i<*currSample; i++) {
            if (*graphicsF == 1) { // graphicsFlag on
                cpuGraphic(&cpuDifference[i]);
            } else { // graphicsFlag off
                printf("         %.2f\n", cpuDifference[i]); 
            }
        }
    }
    for (int k=0; k<samples-(*currSample); k++) {
        printf("\n");
    }
    separator();
}


/*******************
 * Function: displaySystem(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function prints the system information based on the information read from the pipe 
 * file descriptor.
*******************/
void displaySystem(int pipefd) {

    struct utsname buffer;
    if (read(pipefd, &buffer, sizeof(struct utsname)) == -1) {
        fprintf(stderr, "Error reading system info\n");
    }

    printf("### System Information ###\n");
    printf("System Name = %s\n", buffer.sysname);
    printf("Machine Name = %s\n", buffer.nodename);
    printf("Version = %s\n", buffer.version);
    printf("Release = %s\n", buffer.release);
    printf("Architecture = %s\n", buffer.machine);
    separator();
}


/*******************
 * Function: handler(int code)
 * Parameters: int code
 * Returns: void (N/A)
 * 
 * This function provides a handler to handle interceptions from incoming signals from 
 * the user. It can detect Ctrl-z and Ctrl-c signal codes. If Ctrl-z is detected, it will 
 * simply skip to the next iteration and print a message that indicates that the signal has 
 * been caught. With Ctrl-c, the program will ask the user whether they want to terminate 
 * the program. If the user types 'Y' or 'yes', the program will terminate. If the user 
 * types 'N' or 'no', then the program will continue. All other character and/or string of 
 * characters will not be recognized and the program will keep asking the question until 
 * either the program is forcefully terminated or an appriorate input is detected. 
*******************/
void handler(int code) {
    if (code == SIGTSTP) {
        printf("\nCtrl-z caught!\n");
        return;
    } else if (code == SIGINT) {
        bool found = false;
        char input[BUFFER_SIZE];
        terminate();
        scanf("%s", input);
        while (!found) {
            if (strncmp("yes", input, 3) == 0 || strncmp("no", input, 2) == 0 || strncmp("Y", input, 1) == 0 
                || strncmp("N", input, 1) == 0) {
                found = true;
               } else {
                wrong_input();
                scanf("%s", input);
               }
        }
        if (strncmp("yes", input, 3) == 0 || strncmp("Y", input, 1) == 0) {
            exit(0);
        }
        continuation(); // since we know that either N or no was received
    }
}


/*******************
 * Function: main()
 * Parameters: int argc, char *argv[] - number of arguments, command line arguments used
 * Returns: integer - usually 0 if everything executed as expected with no errors
 * 
 * This function is the main function begins the main process/parent process. It displays 
 * the desired information concurrently to the user depending on the command line arguments provided. 
 * In this specific implementation, positional arguments for --samples and --tdelay are possible 
 * but they BOTH have to be provided in order for the program to work as intended.
 * If only a number (e.g. 8) is provided, the program could crash. If --samples
 * and --tdelay are provided with an '=' and a digit right after, samples and 
 * tdelay will update accordingly. If no digit is specified, then default values
 * of 10 samples and 1 sec delay will remain. The rest of the feature flags will be
 * detected if typed correctly as a command line argument and will flip the flag 
 * sign from off to on. This main pocess also allows the user to essentially skip to the
 * next iteration of the samples by pressing ctrl-z or terminate the program by pressing ctrl-c.
*******************/
int main(int argc, char *argv[]) {
    
    // signal handler
    struct sigaction newact;
    newact.sa_handler = handler;
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);
    sigaction(SIGINT, &newact, NULL);  // ctrl-c
    sigaction(SIGTSTP, &newact, NULL); // ctrl-z

    // parent process id
    int pid = getpid();

    int userF = 0, systemF = 0, graphicsF = 0, sequentialF = 0; // all flags are OFF by default
    int samples = 10; // default value
    int tdelay = 1; // default value
    int sampleFlag = 0, delayFlag = 0;
    int count = 2;

    char *givenSamples;
    char *givenDelay;
    const char assign = '=';

    for (int i=1; i<argc; i++) {
        if (strncmp("--system", argv[i], 8) == 0) {
            systemF = 1;
        } else if (strncmp("--users", argv[i], 6) == 0) {
            userF = 1;
        } else if (strncmp("--graphics", argv[i], 10) == 0) {
            graphicsF = 1;
        } else if (strncmp("--sequential", argv[i], 12) == 0) {
            sequentialF = 1;
        } else if (strncmp("--samples", argv[i], 9) == 0) {
            sampleFlag = 1;
            givenSamples = strchr(argv[i], assign);
            if (strcmp(givenSamples, "=") == 0) {
                continue;
            } else {
                samples = atoi((givenSamples+1));
            }
        } else if (strncmp("--tdelay", argv[i], 8) == 0) {
            delayFlag = 1;
            givenDelay = strchr(argv[i], assign);
            if (strcmp(givenDelay, "=") == 0) {
                continue;
            } else {
                tdelay = atoi((givenDelay+1));
            }
        }
    }

    // positional arguments (specify both in numbers or it wont work)
    count = count + systemF + userF + graphicsF + sequentialF;
    if (sampleFlag == 0 && delayFlag == 0 && argc > count ) {
        samples = atoi((argv[argc-2]));
        tdelay = atoi((argv[argc-1]));
    }

    // previous a1 display parameters/arguments/variables
    int memory_index = 0;
    int users_index = 1;
    int proc_index = 2;
    int system_index = 3;

    int currSample = 0;
    float physical[samples];
    float virtual[samples];
    float cpuPercent[samples+1][2];
    float cpuDifference[samples];

    cpuPercent[0][0] = 0;
    cpuPercent[0][1] = 0;
    currSample++;

    while (currSample <= samples) {

        int main_pipe[FUNC_NUM][2]; // 4 print functions
        int mem_pipe[FUNC_NUM][2];  // memory function (main process (parents) + all other processes)
        
        int close_status;
        int pid_tracker[5];
        int current_process;
        int curr_proc_index = 0;

        for (int i=0; i<FUNC_NUM; i++) {
            pid_tracker[i] = 0;
            if (pipe(main_pipe[i]) == -1) {
                fprintf(stderr, "Error setting up main_pipe\n");
            }
            if (pipe(mem_pipe[i]) == -1) {
                fprintf(stderr, "Error setting up mem_pipe\n");
            }
        }

        for (int i=0; i<FUNC_NUM; i++) {
            current_process = getpid();
            if (current_process == pid) {
                pid_tracker[i] = fork();
                if (pid_tracker[i] == -1) {
                    fprintf(stderr, "Error occurred during fork\n");
                }
            }
        }

        // close fds that are not used
        // determine which child processes are running
        while (pid_tracker[curr_proc_index] != 0) { 
            curr_proc_index++;  // if 0, then we found the child
        }

        // case where current process is the main process
        // close all writing pipes
        if (curr_proc_index == FUNC_NUM) {
            int p = 0;
            while (p < FUNC_NUM) {
                close(main_pipe[p][1]);
                close(mem_pipe[p][1]);
                p++;
            }
        }

        // case where child is not the main process
        if (curr_proc_index != FUNC_NUM) {
            for (int i=0; i < FUNC_NUM; i++) {
                // reading fds
                close(main_pipe[i][0]);
                close(mem_pipe[i][0]);
            }
            for (int i=0; i < FUNC_NUM; i++) {
                // writing fds
                if (i != curr_proc_index) {
                    close(main_pipe[i][1]);
                    close(mem_pipe[i][1]);
                }
            }
        }

        if (curr_proc_index == memory_index) {
            passMemory(mem_pipe[memory_index][1]);
            passRam(main_pipe[memory_index][1]);
            if ((close_status = close(main_pipe[memory_index][1])) == -1) {
                fprintf(stderr, "Error closing pipe fd for memory\n");
                exit(EXIT_FAILURE);
            }
            exit(0); // same as EXIT_SUCCESS
        }

        if (curr_proc_index == users_index) {
            passMemory(mem_pipe[users_index][1]);
            passUsers(main_pipe[users_index][1]);
            if ((close_status = close(main_pipe[users_index][1])) == -1) {
                fprintf(stderr, "Error closing pipe fd for users\n");
                exit(EXIT_FAILURE);
            }
            exit(0); // same as EXIT_SUCCESS
        }

        if (curr_proc_index == proc_index) {
            passMemory(mem_pipe[proc_index][1]);
            passCpu(main_pipe[proc_index][1]);
            if ((close_status = close(main_pipe[proc_index][1])) == -1) {
                fprintf(stderr, "Error closing pipe fd for cpu\n");
                exit(EXIT_FAILURE);
            }
            exit(0); // same as EXIT_SUCCESS
        }

        if (curr_proc_index == system_index) {
            passMemory(mem_pipe[system_index][1]);
            passSystem(main_pipe[system_index][1]);
            if ((close_status = close(main_pipe[system_index][1])) == -1) {
                fprintf(stderr, "Error closing pipe fd for system\n");
                exit(EXIT_FAILURE);
            }
            exit(0); // same as EXIT_SUCCESS
        }

        if (curr_proc_index == FUNC_NUM) {
            clear();
            header(&samples, &tdelay);
            displayMemory(mem_pipe);
            if (userF == 0) {
                displayRam(main_pipe[memory_index][0], physical, virtual, &currSample, samples, 
                &graphicsF, &sequentialF);
            }
            if (systemF == 0) {
                displayUsers(main_pipe[users_index][0]);
            }
            if (userF == 0) {
                displayCpu(main_pipe[proc_index][0], &currSample, samples, cpuDifference, cpuPercent, 
                &graphicsF, &sequentialF);
            }
            if (userF == 0 && systemF == 0) {
                displaySystem(main_pipe[system_index][0]);
            }
        }
        currSample++;
        sleep(tdelay);
    }

    return 0;
}
