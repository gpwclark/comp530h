#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include "urrsched.h" /* used by both kernel module and user program */

struct sched_param newParams = {.sched_priority = 1};
int main (int argc, char* argv[]){
    int setSched = -1;
    int getSched = -1;
    int pidtosched = -1;
    if(argc > 1 && (pidtosched = atoi(argv[1]) ) > 0 ){//We need a pid as the arf
        getSched = sched_getscheduler(pidtosched); 
        perror("Trying to Schedule given pid: ");
        if(getSched == -1){// We had an error
            fprintf(stderr, "Exiting on error\n");
            exit(GENERR);
        }
        //now set the scheduling policy
        if(getSched != SCHED_RR){
            fprintf(stderr, "Does not have a SCHED_RR policy\n");
            sched_setscheduler(pidtosched, SCHED_RR, &newParams);
        }
        else{
            fprintf(stderr, "Already has SCHED_RR policy\n");
        }
    }
    return 0;
}
