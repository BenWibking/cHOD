#include "read_hdf5.h"

int main(void)
{
  time_t now = time(NULL);

  printf("Computing HOD from halos.hdf5.\n");

  populate_hod(2, 0.5, 12.5, 12.5, 13.5, 1.0, 42);

  time_t then = time(NULL);
  double diff = difftime(then, now);

  printf("Time: %f \n", diff);
}
