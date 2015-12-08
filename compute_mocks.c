#include "read_hdf5.h"

int main(void)
{
  time_t now = time(NULL);

  printf("Computing HOD from halos.hdf5.\n");

  double redshift = 0.57;
  double Omega_m = 0.528806190857137;

  populate_hod(2, 0.5, 13.4, 0.6, 14.3, 1.5, 42, Omega_m, redshift);

  time_t then = time(NULL);
  double diff = difftime(then, now);

  printf("Time: %f \n", diff);

  return 0;
}
