
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "urrsched.h" /* used by both kernel module and user program */

int fp, childPID;
char the_file[256] = "/sys/kernel/debug/";
char call_buf[MAX_CALL];  /* assumes no bufferline is longer */
char resp_buf[MAX_RESP];  /* assumes no bufferline is longer */

void do_syscall(char *call_string);

int main (int argc, char* argv[])
{
	int i;
	int rc = 0;

	/* Open the file */

	strcat(the_file, dir_name);
	//fprintf(stdout, "dir_name %s\n", dir_name);
	strcat(the_file, "/");
	strcat(the_file, file_name);
	//fprintf(stdout, "file_name %s\n", file_name);
	if ((fp = open (the_file, O_RDWR)) == -1) {
		fprintf (stderr, "error opening %s\n", the_file);
		exit (-1);
	}

    printf("CALLERCYCLESL %llu\n", CALLERCYCLESL);
	int my_pid = getpid();
    char* argS = malloc(sizeof(char) * MAX_CALL);
	i = 1;
	while(1){
		if(i < argc){
			strcat(argS, argv[i]);
		}
		if(i < argc - 1){
			strcat(argS, " ");
		}
		else{break;}
		i++;
	}
	strcat(argS, "\0");
    ///////////////
    int ereturn = 0;
    ////we need to wait on the event
    char *vpargs[] = {
        "call_create_HW3",
        "event_wait",
        "0",
        "0",
        (char *) NULL,
    };
    childPID = fork(); //fork and get the child_PID
    int status = 0;
    if(childPID){//This is the Parent
        waitpid(-1, &status, WUNTRACED);
    }
    else{//tell the child to wait on event, the parent waits on the child
        ereturn = execvpe(vpargs[0], vpargs);
    }
    //Done waiting so lets make the syscall that we are supposed to
	fprintf(stdout, "Process %d calls urrsched with: %s\n",my_pid, argS);
	do_syscall(argS);
	fprintf(stdout, "Module urrsched returns %s to PID %d\n", resp_buf, my_pid);
    //For testing busy wait
    long long unsigned int counter = 0;
    while(ereturn != -1){
        counter++;
        if(counter > CALLERCYCLESL)
            printf("counter == %lld", counter);
            break;
    }
	close (fp);
	free(argS);
    return 0;
} /* end main() */

void do_syscall(char *call_string)
{
  int rc;

  strcpy(call_buf, call_string);

  rc = write(fp, call_buf, strlen(call_buf) + 1);
	if (rc == -1) {
		fprintf (stderr, "error writing %s\n%s\n", the_file, strerror(errno));

		fflush(stderr);
		exit (-1);
	}

	rc = read(fp, resp_buf, sizeof(resp_buf));
	if (rc == -1) {
		fprintf (stderr, "error reading %s\n", the_file);
		fflush(stderr);
		exit (-1);
	}
}

