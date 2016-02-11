#include "read_hdf5.h"

int main(int argc, char *argv[])
{
  if(argc != 12) {
    printf("%d usage: ./compute_mocks Omega_m redshift siglogM logMmin logM0 logM1 alpha boxsize seed [halo catalog file] [galaxy mock file]\n",argc);
    return -1;
  }

  double Omega_m = strtod(argv[1], NULL);
  double redshift = strtod(argv[2], NULL);

  double siglogM = strtod(argv[3], NULL);
  double logMmin = strtod(argv[4], NULL);
  double logM0 = strtod(argv[5], NULL);
  double logM1 = strtod(argv[6], NULL);
  double alpha = strtod(argv[7], NULL);

  double boxsize = strtod(argv[8], NULL);

  int seed = atoi(argv[9]);

  char *halo_file, *output_file;
  size_t halo_file_ssize, output_file_ssize;

  halo_file_ssize = sizeof(char)*(strlen(argv[10]) + 1);
  output_file_ssize = sizeof(char)*(strlen(argv[11]) +1 );

  halo_file = malloc(halo_file_ssize);
  output_file = malloc(output_file_ssize);
  snprintf(halo_file, halo_file_ssize, "%s", argv[10]);
  snprintf(output_file, output_file_ssize, "%s", argv[11]);

  printf("Computing HOD from %s\n", halo_file);
  printf("Saving to output file %s\n", output_file);

  populate_hod(siglogM, logMmin, logM0, logM1, alpha, \
	       seed, Omega_m, redshift,	boxsize, halo_file, output_file);

  return 0;
}
