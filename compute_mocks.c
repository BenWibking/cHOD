#include <stdio.h>
#include <stdlib.h>
#include "hod.c"
#include <time.h>

int main(void)
{
  time_t now = time(NULL);

  populate_hod(2, "halos.hdf5", 0.5, 12.5, 12.5, 13.5, 1.0, 42);

  time_t then = time(NULL);
  double diff = difftime(now, then);

  printf("Time: %f \n", diff);
}
