#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <math.h>
#include <sys/sysinfo.h>

#define KILOTOGIGA (1/pow(1024, 3)) // kilobytes to gigabytes conversion multiplyer constant


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
long int memoryUsage() {
    struct rusage memory;
    if (getrusage(RUSAGE_SELF, &memory) != 0) {
        fprintf(stderr, "Error occurred while using getrusage()\n");
    }
    return memory.ru_maxrss;
}


/*******************
 * Function: memoryAccess()
 * Parameters: struct sysinfo memory
 * Returns: float * - a pointer to a float array of size 4
 * 
 * This function returns a pointer to a 4-block size array that stores
 * the ram, used ram, virtual ram, and used virtual ram memory usage of the 
 * current machine by taking in a sysinfo struct.
*******************/
float *memoryAccess() {

    struct sysinfo memory;
    sysinfo(&memory);

    float memoryArray[4];
    float *mem_ptr;
    mem_ptr = &memoryArray[0];

    float totalRam = memory.totalram*KILOTOGIGA;
    float usedRam = totalRam - memory.freeram*KILOTOGIGA;
    float totalVirtRam = memory.totalram*KILOTOGIGA + memory.totalswap*KILOTOGIGA;
    float usedVirtRam = totalVirtRam - (memory.freeram*KILOTOGIGA + memory.freeswap*KILOTOGIGA);

    memoryArray[0] = usedRam;
    memoryArray[1] = totalRam;
    memoryArray[2] = usedVirtRam;
    memoryArray[3] = totalVirtRam;

    return mem_ptr;
}


int cpuCores() {

    char buffer[255];
    int cores = -1;
    FILE *cpuStats;
    cpuStats = fopen("/proc/stat", "r");

    if (cpuStats == NULL) {
        fprintf(stderr, "Error opening file /proc/stat\n");
    }

    while (fscanf(cpuStats, "%s", buffer) != EOF) {
        if (strncmp(buffer, "cpu", 3) == 0) {
            cores++;
        }
    }

    fclose(cpuStats);
    return cores;
}


void passMemory(int pipefd) {

    long int memory = memoryUsage();
    if (write(pipefd, &memory, sizeof(long)) == -1) {
        fprintf(stderr, "Error writing to pipe (Memory)\n");
    }
}


void passRam (int pipefd) {

    float *mem_ptr;
    mem_ptr = memoryAccess();

    Memory stats;
    stats.usedRam = mem_ptr[0];
    stats.totalRam = mem_ptr[1];
    stats.usedVirtRam = mem_ptr[2];
    stats.totalVirtRam = mem_ptr[3];

    if (write(pipefd, &stats, sizeof(Memory)) == -1) {
        fprintf(stderr, "Error writing to pipe (Ram)\n");
    }
}

// changed the approach for it to be easier to write 
void passUsers(int pipefd) {

    struct utmp *activeUsers;
    setutent(); // rewinds the file pointer to the beginning of the utmp file
    activeUsers = getutent();   // reads line from current file position pointer
    while (activeUsers != NULL) {
        if (activeUsers->ut_type == USER_PROCESS) {
            if (write(pipefd, activeUsers, sizeof(struct utmp)) == -1) {
                fprintf(stderr, "Error write utmp struct\n");
            }
        }
        activeUsers = getutent();   // acts as loop increment
    }
}


void passCpu(int pipefd) {

    int cores;
    Procutil cputil;
    FILE *cpu;

    cpu = fopen("/proc/stat", "r");
    if (cpu == NULL) {
        fprintf(stderr, "Error opening file\n");
    }

    long total = 0;
    long utilization;
    long user, nice, system, idle, iowait, irq, softirq;
    char core[4];

    fscanf(cpu, "%s %lu %lu %lu %lu %lu %lu %lu", core, &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    total = user + nice + system + idle + iowait + irq + softirq;
    utilization = total - idle;

    fclose(cpu);

    cores = cpuCores();
    cputil.count = cores;
    cputil.utilization = utilization;
    cputil.total = total;

    if (write(pipefd, &cputil, sizeof(Procutil)) == -1) {
        fprintf(stderr, "Error writing to pipe (cpu) \n");
    }
}


void passSystem(int pipefd) {

    struct utsname buffer;
    if (uname(&buffer) < 0) {
        fprintf(stderr, "Error getting utsname struct info\n");
        exit(EXIT_FAILURE);
    }

    if (write(pipefd, &buffer, sizeof(struct utsname)) == -1) {
        fprintf(stderr, "Error writing to pipte (system info)\n");
    }
}
