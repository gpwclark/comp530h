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

int fdin;
unsigned long c = 0;
char *src;
struct stat statbuf;
int i, j;
int max_idx;

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


  /* open the input file */
  if ((fdin = open (argv[1], O_RDONLY)) < 0)
      errx (-1, "can't open input for reading");
  /* find size of input file */
  if (fstat (fdin,&statbuf) < 0)
      errx (-1,"fstat error");
  /* mmap the input file */
  if ((src = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == (caddr_t) -1) 
      errx (-1, "mmap error for input");

  do_syscall("vmlogger_new");
  fprintf(stdout, "Module vmlogger returns %s", resp_buf);

  if(strncmp(a[1] , "BigFileRandom", sizeof("BigFileRandom") == 0){
          do_mmap_stuff_rand();
  }
  else if(strncmp(a[1] , "BigFileSequential", sizeof("BigFileSequential") == 0){
          do_mmap_stuff_seq();
  }
  else if(strncmp(a[1] , "BigFileStride", sizeof("BigFileStride") == 0){
          do_mmap_stuff_stride();
  }
  //else do nothing
  //
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

void do_mmap_stuff_seq(){
    /* read at random from mapped file, compute a “checksum” */
    fprintf(stdout, "Reading %d bytes from mapped file in sequence\n", statbuf.st_size);
    max_idx = statbuf.st_size - 2;
    for (i = 0; i < statbuf.st_size; i++){
        j = i;
        c += (*(src+j)) >> 2;
    }
    fprintf(stdout, "Read %d bytes, sum %lu\n", i-1, c); 
}
void do_mmap_stuff_stride(){
    /* read at random from mapped file, compute a “checksum” */
    fprintf(stdout, "Reading %d bytes from mapped file with a hefty stride\n", statbuf.st_size);
    max_idx = statbuf.st_size - 2;
    for (i = 0; i < statbuf.st_size; i++){
        j = (i * STRIDE) % max_idx;
        c += (*(src+j)) >> 2;
    }
    fprintf(stdout, "Read %d bytes, sum %lu\n", i-1, c); 
}
void do_mmap_stuff_rand(){
    /* read at random from mapped file, compute a “checksum” */
    fprintf(stdout, "Reading %d bytes from mapped file absolutely randomly\n", statbuf.st_size);
    max_idx = statbuf.st_size - 2;
    for (i = 0; i < statbuf.st_size; i++){
        j = random() % max_idx;
        c += (*(src+j)) >> 2;
    }
    fprintf(stdout, "Read %d bytes, sum %lu\n", i-1, c); 
}
