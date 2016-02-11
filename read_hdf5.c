#include "read_hdf5.h"

void* read_halo_hdf5(char filename[], char dataset_name[], size_t *len) {
  /* open HDF5 file*/
  hid_t halo_tid;
  hid_t file_id, dataset, space;
  hsize_t dims[2];

  file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  if (file_id < 0) {
    H5Eprint(H5E_DEFAULT, stderr);
    exit(1); /* by default, HDF5 lib will print error to console */
  }

  dataset = H5Dopen(file_id, dataset_name, H5P_DEFAULT);
  if (dataset < 0) {
    H5Eprint(H5E_DEFAULT, stderr);
    exit(1);
  }

  space = H5Dget_space(dataset);
  H5Sget_simple_extent_dims(space, dims, NULL);
  hostDMH *data = (hostDMH*) malloc(dims[0]*sizeof(hostDMH));

  halo_tid = H5Tcreate(H5T_COMPOUND, sizeof(hostDMH));
  H5Tinsert(halo_tid, "mass", HOFFSET(hostDMH,mass),H5T_NATIVE_FLOAT);
  H5Tinsert(halo_tid, "x", HOFFSET(hostDMH,X), H5T_NATIVE_FLOAT);
  H5Tinsert(halo_tid, "y", HOFFSET(hostDMH,Y), H5T_NATIVE_FLOAT);
  H5Tinsert(halo_tid, "z", HOFFSET(hostDMH,Z), H5T_NATIVE_FLOAT);

  if (H5Dread(dataset, halo_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0) {
    H5Eprint(H5E_DEFAULT, stderr);
    exit(1);
  }

  *len = dims[0];

  H5Fclose(file_id);

  return (void*)data;
}
