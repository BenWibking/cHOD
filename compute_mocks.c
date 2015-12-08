#include "read_hdf5.h"

int main(int argc, char *argv[])
{
  time_t now = time(NULL);

  printf("Computing HOD from halos.hdf5.\n");

  double redshift = 0.57;
  double Omega_m = 0.528806190857137;

  double siglogM = strtod(argv[1], NULL);
  double logMmin = strtod(argv[2], NULL);
  double logM0 = strtod(argv[3], NULL);
  double logM1 = strtod(argv[4], NULL);
  double alpha = strtod(argv[5], NULL);

  populate_hod(2, siglogM, logMmin, logM0, logM1, alpha, 42, Omega_m, redshift);

  time_t then = time(NULL);
  double diff = difftime(then, now);

  printf("Time: %f \n", diff);

  return 0;
}
