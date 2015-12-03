#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

double NFW_CDF_sampler(double c_vir, gsl_rng *r)
{
  /* This function calculates the radial CDF for a halo of a given concentration and implements a random sampling for the CDF*/

  double CDF[1000];
  size_t i;
  double prefac = 1.0 / ( log( 1.0 + c_vir ) - c_vir / ( 1.0 + c_vir ) ); /* Prefactor 1/A(c_vir) */

  for(i=0; i<1000; i++)
    {
      double x = (double)i / 1000.0;
      CDF[i] = prefac * ( log( 1.0 + x * c_vir ) - x * c_vir / ( 1.0 + x*c_vir ) );
    }

  double rando = gsl_rng_uniform(r);

  size_t j;

  for(j=0; j<1000; j++)
    {
      if(CDF[j]>rando)
	{
	  printf("j=%zd CDF[j]=%g random=%f\n",j,CDF[j],rando);
	  break;
	}
    }
  double R_frac = ((double)j) / 1000.0;
  printf("R_frac %f\n", R_frac);
  return R_frac;
}
