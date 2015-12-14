#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <time.h>

#include "hdf5.h"
#include "hdf5_hl.h"

typedef struct halo
{
  float mass;
  float X;
  float Y;
  float Z;
} hostDMH;

typedef struct HODgal
{
  float X;
  float Y;
  float Z;
} galaxy;

void populate_hod(double siglogM, double logMmin, double logM0, double logM1, double alpha, unsigned long int seed, double Omega_m, double redshift, char *input_fname, char *output_fname);
double NFW_CDF_sampler(float * restrict CDF, gsl_rng *r);
void* read_halo_hdf5(char infile[],char dataset_name[],size_t *len);
herr_t write_gal_hdf5(char filename[], char dataset_name[], size_t len, galaxy* data);
