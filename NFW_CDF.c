#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

double NFW_CDF_sampler(double c_vir, unsigned long int seed);

int main(void)
{
  double frac;
  frac = NFW_CDF_sampler(10.0, 42);
  printf("frac %f \n", frac);
}

double NFW_CDF_sampler(double c_vir, unsigned long int seed)
{
  /* This function calculates the radial CDF for a halo of a given concentration and implements a random sampling for the CDF*/

  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, seed); /* Seeding random distribution */
 
  double CDF[1000];
  int i;
  double prefac = 1.0 / ( log( 1.0 + c_vir ) - c_vir / ( 1.0 + c_vir ) ); /* Prefactor 1/A(c_vir) */

  for(i=0; i<1000; i++)
    {
      double x = (double)i / 1000.0;
      CDF[i] = prefac * ( log( 1.0 + x * c_vir ) - x * c_vir / ( 1.0 + x*c_vir ) );
    }

  double rando = gsl_rng_uniform(r);

  int j;

  for(j=0; j<1000; j++)
    {
      if(CDF[j]>rando)
	{
	  break;
	}
    }
  double R_frac = j/1000.0;

  gsl_rng_free(r);
 
  return R_frac;
}
