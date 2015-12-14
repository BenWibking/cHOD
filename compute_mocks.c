#include "read_hdf5.h"

int main(int argc, char *argv[])
{
  if(argc != 11) {
    printf("%d usage: ./compute_mocks Omega_m redshift siglogM logMmin logM0 logM1 alpha seed [halo catalog file] [galaxy mock file]\n",argc);
    return -1;
  }

  double Omega_m = strtod(argv[1], NULL);
  double redshift = strtod(argv[2], NULL);

  double siglogM = strtod(argv[3], NULL);
  double logMmin = strtod(argv[4], NULL);
  double logM0 = strtod(argv[5], NULL);
  double logM1 = strtod(argv[6], NULL);
  double alpha = strtod(argv[7], NULL);
  int seed = atoi(argv[8]);

  char *halo_file, *output_file;

  halo_file = malloc(sizeof(char)*strlen(argv[9]));
  output_file = malloc(sizeof(char)*strlen(argv[10]));
  sprintf(halo_file,"%s",argv[9]);
  sprintf(output_file,"%s",argv[10]);

  printf("Computing HOD from %s.\n", halo_file);

  populate_hod(siglogM, logMmin, logM0, logM1, alpha, \
	       seed, Omega_m, redshift,	halo_file, output_file);

  return 0;
}
