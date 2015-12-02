#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1000
#define Omega_Matter 0.29 /*Omega Matter*/
#define redshift 0.57 /*Box Redshift*/

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


void* read_halo_hdf5(char infile[],char dataset_name[],size_t *len);
herr_t write_gal_hdf5(char filename[], char dataset_name[], size_t len, galaxy* data);
