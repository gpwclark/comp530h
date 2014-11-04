
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "urrsched.h" /* used by both kernel module and user program */


int fp_usersync;
char the_file_usersync[256] = "/sys/kernel/debug/";
char call_buf_usersync[MAX_CALL];  /* assumes no bufferline is longer */
char resp_buf_usersync[MAX_RESP];  /* assumes no bufferline is longer */

char dir_name_usersync[] = "usersync";
char file_name_usersync[] = "call";

void do_syscall_usersync(char *call_string);

void main_usersync()
{
	int i;
	int rc = 0;

	/* Open the file */

	strcat(the_file_usersync, dir_name_usersync);
	//fprintf(stdout, "dir_name %s\n", dir_name);
	strcat(the_file_usersync, "/");
	strcat(the_file_usersync, file_name_usersync);
	//fprintf(stdout, "file_name %s\n", file_name);
	if ((fp_usersync = open (the_file_usersync, O_RDWR)) == -1) {
		fprintf (stderr, "error opening %s\n", the_file_usersync);
		exit (-1);
	}

	int my_pid = getpid();
	char* argS = malloc(sizeof(char) * MAX_CALL);
    strcat(argS, "event_wait 0 0");
	strcat(argS, "\0");
	//fprintf(stdout, "Strcmp, %i", strcmp("event_create myev", argS) );
	fprintf(stdout, "Process %d calls usersync with: %s\n",my_pid, argS);
	do_syscall_usersync(argS);
	fprintf(stdout, "Module usersync returns %s to PID %d\n", resp_buf_usersync, my_pid);

	close (fp_usersync);
	free(argS);
} /* end main() */

void do_syscall_usersync(char *call_string)
{
  int rc;

  strcpy(call_buf_usersync, call_string);

  rc = write(fp_usersync, call_buf_usersync, strlen(call_buf_usersync) + 1);
	if (rc == -1) {
		fprintf (stderr, "error writing %s\n", the_file_usersync);
		fflush(stderr);
		exit (-1);
	}

	rc = read(fp_usersync, resp_buf_usersync, sizeof(resp_buf_usersync));
	if (rc == -1) {
		fprintf (stderr, "error reading %s\n", the_file_usersync);
		fflush(stderr);
		exit (-1);
	}
}
int fp, childPID;
char the_file[256] = "/sys/kernel/debug/";
char call_buf[MAX_CALL];  /* assumes no bufferline is longer */
char resp_buf[MAX_RESP];  /* assumes no bufferline is longer */

void do_syscall(char *call_string);

int main (int argc, char* argv[])
{
	int i;
	int rc = 0;
	int my_pid = getpid();

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
    main_usersync();
    //Done waiting so lets make the syscall that we are supposed to
	fprintf(stdout, "Process %d calls urrsched with: %s\n",my_pid, argS);
	do_syscall(argS);
	fprintf(stdout, "Module urrsched returns %s to PID %d\n", resp_buf, my_pid);
    //For testing busy wait
    time_t start = time(0);
    while(1){
        time_t now = time(0);
        double seconds = difftime(now, start);
        if(seconds > 10)
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

