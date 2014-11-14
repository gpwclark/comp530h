#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "vmlogger.h" /* used by both kernel module and user program */

int fp;
char the_file[256] = "/sys/kernel/debug/";
char call_buf[MAX_CALL];  /* assumes no bufferline is longer */
char resp_buf[MAX_RESP];  /* assumes no bufferline is longer */

void do_syscall(char *call_string);
void do_mmap_stuff();

void main (int argc, char* argv[])
{
  int i;
  int rc = 0;

  /* Open the file */

  strcat(the_file, dir_name);
  strcat(the_file, "/");
  strcat(the_file, file_name);

  if ((fp = open (the_file, O_RDWR)) == -1) {
      fprintf (stderr, "error opening %s\n", the_file);
      exit (-1);
  }

  int my_pid = getpid();
  fprintf(stdout, "System call getpid() returns %d\n", my_pid);

  do_syscall("vmlogger");
  fprintf(stdout, "Module vmlogger returns %s", resp_buf);
  do_mmap_stuff();

  close (fp);
} /* end main() */

void do_syscall(char *call_string)
{
  int rc;

  strcpy(call_buf, call_string);

  rc = write(fp, call_buf, strlen(call_buf) + 1);
  if (rc == -1) {
     fprintf (stderr, "error writing %s\n", the_file);
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

void do_mmap_stuff(){
    int fdin;
    unsigned long c = 0;
    char *src;
    struct stat statbuf;
    int i, j;
    int max_idx;
    /* open the input file */
    if ((fdin = open ("BigFile", O_RDONLY)) < 0)
        errx (-1, "can't open input for reading");
    /* find size of input file */
    if (fstat (fdin,&statbuf) < 0)
        errx (-1,"fstat error");
    /* mmap the input file */
    if ((src = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == (caddr_t) -1) 
        errx (-1, "mmap error for input");
        /* read at random from mapped file, compute a “checksum” */
    fprintf(stdout, "Reading %d bytes from mapped file\n", statbuf.st_size);
    max_idx = statbuf.st_size - 2;
    for (i = 0; i < statbuf.st_size; i++){
        j = random() % max_idx;
        c += (*(src+j)) >> 2;
    }
    fprintf(stdout, "Read %d bytes, sum %lu\n", i-1, c); 
}
