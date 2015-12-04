#include "read_hdf5.h"

#define Mpc_to_cm pow(3.0856, 24.0) /*Conversion factor from Mpc to cm */
#define Msun_to_g pow(1.989, 33.0) /*Conversion factor from Msun to grams*/
#define G pow(6.672, -8.0) /*Universal Gravitational Constant in cgs units*/
#define Hubble pow(3.2407789, -18.0) /*Hubble's constant h/sec*/
#define rho_crit (3.0*pow(Hubble, 2.0) / (8.0 * M_PI * G)) * (pow(Mpc_to_cm, 3.0) / Msun_to_g) /*Cosmological Critical Density in Msun h^2 / Mpc^3 */
#define INDEX3(i,j) (i*3 + j)

/* These functions populate halos with HOD galaxies from an HDF5 halo catalog */
/* The outputs specify galaxy positions, and velocities (host mass and satellite/central identification have been dropped for speed and space considerations) */
/* For now just positions */

galaxy * find_galaxy_hosts(struct halo halos[], double siglogM, double logMmin, unsigned long int N_h, unsigned long int *Ncen, gsl_rng *r)
{
  /*This function uses the Zehavi 2011 prescription to find the halos that host central galaxies*/

  double * hosts = malloc(N_h * 3 * sizeof(double));
  int i;
  unsigned long j = 0;

  for(i = 0; i < N_h; i++)
    {
      double logM = log10(halos[i].mass); // for now column index is 1 for mass
      double prob = 0.5 * (1.0 + erf( (logM - logMmin) / siglogM) ); /*Mean central occupation or the probability of hosting a central*/
      if(prob > gsl_rng_uniform(r))
	{
	  hosts[INDEX3(j,0)] = halos[i].X;
	  hosts[INDEX3(j,1)] = halos[i].Y;
	  hosts[INDEX3(j,2)] = halos[i].Z;
	  j ++;
	}
    }
  
  *Ncen = j;

  galaxy *host_coords = malloc(j*sizeof(galaxy));

  for(i=0;i<j;i++)
    {
      host_coords[i].X = hosts[INDEX3(i,0)];
      host_coords[i].Y = hosts[INDEX3(i,1)];
      host_coords[i].Z = hosts[INDEX3(i,2)];
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
      
      double logM = log10(halos[i].mass); //Come back to this once hdf5 is figured out
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

  double alpha = 1.62774 - 0.2458*(1.0 + z) + 0.01716*pow(1.0 + z, 2.0); //Redshift and Omega matter don't change for a given box so these should be calculated before hand and stored as global variables
  double beta = 1.66079 + 0.00359*(1.0 + z) - 1.6901*pow(1.0 + z, 0.00417);
  double gamma = -0.02049 + 0.0253*pow(1.0 + z ,-0.1044);

  double D_vir = 200.0, rho_u = rho_crit*O_m;

  double logM = log10(host.mass), x0 = host.X, y0 = host.Y, z0 = host.Z;
  double exponent = alpha + beta*logM*(1.0 + gamma*pow(logM, 2.0)); /* Fit from Correa et al. 2015 */
  double cvir = sqrt(2)*pow(10.0, exponent); /* Approximate factor to rescale Rvir between crit, matter */
  double R_vir = pow((3.0/4.0)*(1/M_PI)*(host.mass/(D_vir*rho_u)), 1.0/3.0);
  
  int j;

  for(j=0; j<N_sat; j++)
    {
      double frac = NFW_CDF_sampler(cvir, r);
      double R = R_vir * frac;
      double phi = 2*M_PI*gsl_rng_uniform(r), costheta = 2*gsl_rng_uniform(r) - 1; /* Sphere point picking */
      double sintheta = sqrt(1 - costheta*costheta);
      double x = R*sintheta*cos(phi)+x0 , y = R*sintheta*sin(phi)+y0 , z = R*costheta+z0; /* Satellite Galaxy Coordinates */
      coords[j].X = x;
      coords[j].Y = y;
      coords[j].Z = z;
    }

  return coords;
}

void populate_hod(int N, double siglogM, double logMmin, double logM0, double logM1, double alpha, unsigned long int seed)
{
  herr_t status;
  size_t NumData,i;
  hostDMH *data;

  unsigned long Ncen;
  unsigned long Nsat;

  if(N != 2)
    {
      fprintf(stderr, "Usage: read_hdf5 [hdf5 file].\n");
      exit(1);
    }

  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, seed); /* Seeding random distribution */

  data = read_halo_hdf5("halos.hdf5","halos",&NumData);
  printf("NumData: %ld\n", NumData);
  for(i=0;i<3;i++){
    printf("%f %f %f %f\n", data[i].mass, data[i].X, data[i].Y, data[i].Z);
  }
  printf("Finding Centrals...\n");
  galaxy *cens; //Central Coordinates
  cens = find_galaxy_hosts(data, siglogM, logMmin, NumData, &Ncen, r);
  printf("NumCens: %lu \n", Ncen);
  for(i=0;i<3;i++){
    printf("%f %f %f\n", cens[i].X, cens[i].Y, cens[i].Z);
  }
  printf("Centrals Found. Finding Satellites...\n");

  int *sats; //Number of Satellites for each halo
  sats = find_satellites(data, siglogM, logMmin, logM0, logM1, alpha, NumData, &Nsat, r);
  printf("NumSat: %lu \n", Nsat);
  galaxy * coords  = malloc(Nsat*sizeof(galaxy)); //Satellite Coordinates
    
  int j,k,l=0;

  for(j=0;j<NumData;j++)
  {
    if(*(sats+j)>0){
      galaxy * halosats = malloc(*(sats+j) * sizeof(galaxy));
      halosats = pick_NFW_satellites(data[j], *(sats+j), Omega_Matter, redshift, r); /*Cosmological Parameters defined in read_hdf5.h file*/
      for(k=0; k<*(sats+j); k++)
	{
	  coords[l].X = halosats[k].X;
	  coords[l].Y = halosats[k].Y;
	  coords[l].Z = halosats[k].Z;
	  l++;
	}
      free(halosats);
    }
  }
  
  free(sats);
  for(i=0;i<3;i++){
    printf("%f %f %f\n", coords[i].X, coords[i].Y, coords[i].Z);
  }

  unsigned long int len = Nsat + Ncen;
  printf("Total Galaxies: %ld \n",len);
  printf("Checkpoint\n");

  galaxy *HODgals = malloc(len*sizeof(galaxy));

  printf("Checkpoint\n");
  //memcpy(HODgals, cens, Ncen);
  for(i=0; i<Ncen;i++)
    {
      HODgals[i].X = cens[i].X;
      HODgals[i].Y = cens[i].Y;
      HODgals[i].Z = cens[i].Z;
    }
  printf("Checkpoint\n");
  free(cens);
  printf("Checkpoint\n");
  //memcpy(HODgals+Ncen, coords, Nsat);

  for(i=0; i<Nsat; i++)
  {
    HODgals[i+Ncen].X = coords[i].X;
    HODgals[i+Ncen].Y = coords[i].Y;
    HODgals[i+Ncen].Z = coords[i].Z;
  }
  printf("Checkpoint\n");
  free(coords);

  char outfile[] = "outputfile.hdf5";

  //snprintf(outfile, "HOD_%f_%f_%f_%f_%f_seed_%i.hdf5", siglogM, logMmin, logM0, logM1, alpha, 42);
  printf("j = %i \n", j);
  printf("Satellites Found. Writing to HDF5 catalog...\n");
  printf("Checkpoint\n");

  status = write_gal_hdf5(outfile, "particles", (size_t)len, HODgals);
  printf("Checkpoint\n");
  
  free(HODgals);
  printf("Checkpoint\n");

  gsl_rng_free(r);

  printf("Done!\n");
}

