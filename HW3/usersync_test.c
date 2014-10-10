#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

double memory[1000][1000];

int main(int argc, char* argv[])
{
  char echo_data[200];  /* warning must be big enough */

  fgets(echo_data, sizeof(echo_data), stdin);
  return 0;
}
