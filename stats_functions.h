#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <utmp.h>


typedef struct memory_use {
    float usedRam;
    float totalRam;
    float usedVirtRam;
    float totalVirtRam;
} Memory;

typedef struct cpu_util {
    int count;
    float utilization;
    float total;
} Procutil;


/*******************
 * Function: memoryUsage()
 * Parameters: N/A
 * Returns: long integer - memory usage in kilobytes of the current process
 * 
 * This function acquires the memory usage for the current process in kilobytes
 * (the value printed on the second line).
*******************/
long int memoryUsage();


/*******************
 * Function: memoryAccess()
 * Parameters: struct sysinfo memory
 * Returns: float * - a pointer to a float array of size 4
 * 
 * This function returns a pointer to a 4-block size array that stores
 * the ram, used ram, virtual ram, and used virtual ram memory usage of the 
 * current machine by taking in a sysinfo struct.
*******************/
float *memoryAccess();


/*******************
 * Function: cpuCores()
 * Parameters: N/A
 * Returns: integer - number of cpu cores
 * 
 * This function opens the file '/proc/stat' and reads line by line until
 * the first column no longer contains the substring "cpu" in order to get
 * the number of cpu cores of the current machine.
*******************/
int cpuCores();


/*******************
 * Function: passMemory(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function writes to the pipe file descriptor the memory usage 
 * of the machine in the current process.
*******************/
void passMemory(int pipefd);


/*******************
 * Function: passRam(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function writes to the pipe file descriptor the ram utilization 
 * of the machine in the current process.
*******************/
void passRam (int pipefd);


/*******************
 * Function: passUsers(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function writes to the pipe file descriptor the active users 
 * of the machine in the current process.
*******************/
void passUsers(int pipefd);


/*******************
 * Function: passCpu(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function writes to the pipe file descriptor the cpu utilization 
 * stats of the machine in the current process.
*******************/
void passCpu(int pipefd);


/*******************
 * Function: passSystem(int pipefd)
 * Parameters: int pipefd
 * Returns: void (N/A)
 * 
 * This function writes to the pipe file descriptor the system information 
 * of the machine in the current process.
*******************/
void passSystem(int pipefd);
