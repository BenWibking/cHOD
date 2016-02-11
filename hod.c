#include "read_hdf5.h"

#define M_PI 3.14159265358979323846
#define Mpc_to_cm 3.0856e24 /*Conversion factor from Mpc to cm */
#define Msun_to_g 1.989e33 /*Conversion factor from Msun to grams*/
#define G 6.672e-8 /*Universal Gravitational Constant in cgs units*/
#define Hubble 3.2407789e-18 /*Hubble's constant h/sec*/
#define rho_crit (3.0*pow(Hubble, 2.0) / (8.0 * M_PI * G)) * (pow(Mpc_to_cm, 3.0) / Msun_to_g) /*Cosmological Critical Density in Msun h^2 / Mpc^3 */
#define INDEX4(i,j) (i*4 + j)

/* These functions populate halos with HOD galaxies from an HDF5 halo catalog */
/* The outputs specify galaxy positions, and velocities (host mass and satellite/central identification have been dropped for speed and space considerations) */
/* For now just positions */

hostDMH * find_galaxy_hosts(struct halo halos[], double siglogM, double logMmin, unsigned long int N_h, unsigned long int *Ncen, gsl_rng *r)
{
  /*This function uses the Zehavi 2011 prescription to find the halos that host central galaxies*/

  float * hosts = malloc(N_h * 4 * sizeof(float));
  int i;
  unsigned long j = 0;

  float f_logMmin = (float)logMmin;
  float f_siglogM = (float)siglogM;

  for(i = 0; i < N_h; i++)
    {
      float logM = (float)log10(halos[i].mass); // for now column index is 1 for mass
      float prob = 0.5 * (1.0 + erf( (logM - f_logMmin) / f_siglogM) ); /*Mean central occupation or the probability of hosting a central*/
      if(prob > gsl_rng_uniform(r))
	{
	  hosts[INDEX4(j,0)] = halos[i].X;
	  hosts[INDEX4(j,1)] = halos[i].Y;
	  hosts[INDEX4(j,2)] = halos[i].Z;
	  hosts[INDEX4(j,3)] = halos[i].mass;
	  j ++;
	}
    }
  
  *Ncen = j;

  hostDMH *host_coords = malloc(j*sizeof(hostDMH));

  for(i=0;i<j;i++)
    {
      host_coords[i].X = hosts[INDEX4(i,0)];
      host_coords[i].Y = hosts[INDEX4(i,1)];
      host_coords[i].Z = hosts[INDEX4(i,2)];
      host_coords[i].mass = hosts[INDEX4(i,3)];
    }

  return host_coords; 
}

int * find_satellites(struct halo halos[], double siglogM, double logMmin, double logM0, double logM1, double alpha, unsigned long int N_h, unsigned long int *Nsat, gsl_rng *r)
{
  /*This function determines how many satellite galaxies each halo has using the same Zehavi 2011 prescription*/

  int i;
  unsigned long j =0;
  int * satellites = malloc(N_h*sizeof(int));

  for(i=0; i < N_h; i++) 
    {
      double M0 = pow(10.0, logM0);
      double M1 = pow(10.0, logM1);
      
      float logM = log10(halos[i].mass); //Come back to this once hdf5 is figured out
      double mean_cen = 0.5 * (1.0 + erf( (logM - logMmin) / siglogM) );
      double mean_sat;
      if(logM < logM0)
	{
	  mean_sat = 0.0; /* Enforcing the satellite cutoff */
	}
      else
	{
	  mean_sat = mean_cen * pow( ( ( halos[i].mass - M0 ) / M1 ), alpha );
	}
      unsigned int N_sat_this_halo = gsl_ran_poisson(r, mean_sat); /* Drawing from Poisson distribution centered at mean_sat to determine Nsat */
      satellites[i] =  N_sat_this_halo;
      j = j + N_sat_this_halo;
    }
  
  *Nsat = j;

  return satellites;
}

galaxy * pick_NFW_satellites(struct halo host, const int N_sat, double O_m, double z, gsl_rng *r)
{
  /* This function determines the spatial distribution of the satellite galaxies */
  /* Galaxies are NFW-distributed using results from Correa et al. 2015 */

  galaxy * coords = malloc(N_sat * sizeof(galaxy));

  double alpha = 1.62774 - 0.2458*(1.0 + z) + 0.01716*pow(1.0 + z, 2.0);
  double beta = 1.66079 + 0.00359*(1.0 + z) - 1.6901*pow(1.0 + z, 0.00417);
  double gamma = -0.02049 + 0.0253*pow(1.0 + z ,-0.1044);

  double D_vir = 200.0, rho_u = rho_crit*O_m;

  double logM = log10(host.mass), x0 = host.X, y0 = host.Y, z0 = host.Z;
  double exponent = alpha + beta*logM*(1.0 + gamma*pow(logM, 2.0)); /* Fit from Correa et al. 2015 */
  double cvir = sqrt(2.0)*pow(10.0, exponent); /* Approximate factor to rescale Rvir between crit, matter */
  double R_vir = pow((3.0/4.0)*(1.0/M_PI)*(host.mass/(D_vir*rho_u)), 1.0/3.0);
  
  int j;

  /* pre-compute NFW profile for this satellite */
  float CDF[1000];
  size_t i;
  double prefac = 1.0 / ( log( 1.0 + cvir ) - (cvir / ( 1.0 + cvir )) ); /* Prefactor 1/A(c_vir) */
  float f_c_vir = (float)cvir;

#pragma simd
  for(i=0; i<1000; i++)
    {
      float x = (float)i / 1000.0;
      CDF[i] = prefac * ( log( 1.0 + x * f_c_vir ) - (x * f_c_vir / ( 1.0 + x*f_c_vir )) );
    }
  
  for(j=0; j<N_sat; j++)
    {
      double frac = NFW_CDF_sampler(&CDF[0], r);
      double R = R_vir * frac;
      double phi = 2.0*M_PI*gsl_rng_uniform(r), costheta = 2.0*gsl_rng_uniform(r) - 1.0; /* Sphere point picking */
      double sintheta = sqrt(1.0 - costheta*costheta);
      double x = R*sintheta*cos(phi)+x0 , y = R*sintheta*sin(phi)+y0 , z = R*costheta+z0; /* Satellite Galaxy Coordinates */
      coords[j].X = x;
      coords[j].Y = y;
      coords[j].Z = z;
    }

  return coords;
}

inline double wrap_periodic(double x, double Lbox)
{
  if((x < Lbox) && (x >= 0.)) {
    return x;
  } else if (x >= Lbox) {
    return (x-Lbox);
  } else if (x < 0.) {
    return (x+Lbox);
  }
}

void populate_hod(double siglogM, double logMmin, double logM0, double logM1, double alpha, unsigned long int seed, double Omega_m, double redshift, double Lbox, char *input_fname, char *output_fname)
{
  herr_t status;
  size_t NumData,i;
  hostDMH *data;

  unsigned long Ncen;
  unsigned long Nsat;

  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, seed); /* Seeding random distribution */

  data = read_halo_hdf5(input_fname,"halos",&NumData);

  /* compute HOD parameters from number density, mass function */  

  hostDMH *cenhalos; //Central Coordinates
  cenhalos = find_galaxy_hosts(data, siglogM, logMmin, NumData, &Ncen, r);
  galaxy * cens = malloc(Ncen*sizeof(galaxy));

  for(i=0; i<Ncen; i++){
    cens[i].X = cenhalos[i].X;
    cens[i].Y = cenhalos[i].Y;
    cens[i].Z = cenhalos[i].Z;
  }
  int *sats; //Number of Satellites for each halo
  sats = find_satellites(cenhalos, siglogM, logMmin, logM0, logM1, alpha, Ncen, &Nsat, r);
  galaxy * coords  = malloc(Nsat*sizeof(galaxy)); //Satellite Coordinates
  int j,k,l=0;

  for(j=0;j<Ncen;j++)
  {
    if(sats[j]>0){
      galaxy * halosats = malloc(sats[j] * sizeof(galaxy));
      halosats = pick_NFW_satellites(cenhalos[j], sats[j], Omega_m, redshift, r); /*Cosmological Parameters defined in read_hdf5.h file*/
      for(k=0; k<sats[j]; k++)
	{
	  coords[l].X = wrap_periodic(halosats[k].X,Lbox);
	  coords[l].Y = wrap_periodic(halosats[k].Y,Lbox);
	  coords[l].Z = wrap_periodic(halosats[k].Z,Lbox);
	  l++;
	}
      free(halosats);
    }
  }
  
  free(cenhalos);
  free(sats);

  unsigned long int len = Nsat + Ncen;

  galaxy *HODgals = malloc(len*sizeof(galaxy));

  for(i=0; i<Ncen;i++)
    {
      HODgals[i].X = cens[i].X;
      HODgals[i].Y = cens[i].Y;
      HODgals[i].Z = cens[i].Z;
    }
  free(cens);
  
  for(i=0; i<Nsat; i++)
  {
    HODgals[i+Ncen].X = coords[i].X;
    HODgals[i+Ncen].Y = coords[i].Y;
    HODgals[i+Ncen].Z = coords[i].Z;
  }
  free(coords);

  printf("Satellites Found. Writing to HDF5 file: %s\n", output_fname);
  status = write_gal_hdf5(output_fname, "particles", (size_t)len, HODgals);
  
  free(HODgals);
 
  gsl_rng_free(r);
}

