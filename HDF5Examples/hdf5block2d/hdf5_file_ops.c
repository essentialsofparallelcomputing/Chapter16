#include "hdf5_file_ops.h"

hid_t create_hdf5_file(const char *filename, MPI_Comm mpi_hdf5_comm);
hid_t open_hdf5_file(const char *filename, MPI_Comm mpi_hdf5_comm);
hid_t create_hdf5_filespace(int ny_global, int nx_global, int ny, int nx,
    int ny_offset, int nx_offset, MPI_Comm mpi_hdf5_comm);
hid_t create_hdf5_memspace(int ny, int nx, int ng);
hid_t create_hdf5_dataset(hid_t file_identifier, hid_t filespace);
hid_t open_hdf5_dataset(hid_t file_identifier);

void hdf5_file_init(int ng, int ny_global, int nx_global, int ny, int nx,
    int ny_offset, int nx_offset, MPI_Comm mpi_hdf5_comm,
    hid_t *memspace, hid_t *filespace){
  // create data descriptors on disk and in memory
  *filespace = create_hdf5_filespace(ny_global, nx_global, ny, nx,
               ny_offset, nx_offset, mpi_hdf5_comm);
  *memspace  = create_hdf5_memspace(ny, nx, ng);
}

hid_t create_hdf5_filespace(int ny_global, int nx_global, int ny, int nx,
    int ny_offset, int nx_offset, MPI_Comm mpi_hdf5_comm){
  // create the dataspace for data stored on disk using the hyperslab call
  hsize_t dims[] = {ny_global, nx_global};
  int ndims = sizeof(dims)/sizeof(hsize_t);

  hid_t filespace = H5Screate_simple(ndims, dims, NULL);

  // figure out the offset into the filespace for the current process
  hsize_t  start[] = {ny_offset, nx_offset};
  hsize_t stride[] = {1,         1};
  hsize_t  count[] = {ny,        nx};

  H5Sselect_hyperslab(filespace, H5S_SELECT_SET,
                start, stride, count, NULL);
  return filespace;
}

hid_t create_hdf5_memspace(int ny, int nx, int ng) {
  // create a memory space in memory using the hyperslab call
  hsize_t dims[] = {ny+2*ng, nx+2*ng};
  int ndims = sizeof(dims)/sizeof(hsize_t);

  hid_t memspace = H5Screate_simple(ndims, dims, NULL);

  // select the real data out of the array
  hsize_t  start[] = {ng,   ng};
  hsize_t stride[] = {1,    1};
  hsize_t  count[] = {ny,   nx};

  H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
                      start, stride, count, NULL);
  return memspace;
}

void hdf5_file_finalize(hid_t *memspace, hid_t *filespace){
  H5Sclose(*memspace);
  *memspace = H5S_NULL;
  H5Sclose(*filespace);
  *filespace = H5S_NULL;
}

void write_hdf5_file(const char *filename, double **data1, hid_t memspace,
    hid_t filespace, MPI_Comm mpi_hdf5_comm) {
  hid_t file_identifier = create_hdf5_file(filename, mpi_hdf5_comm);

  // Create property list for collective dataset write.
  hid_t xfer_plist = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);

  hid_t dataset1 = create_hdf5_dataset(file_identifier, filespace);
  //hid_t dataset2 = create_hdf5_dataset(file_identifier, filespace);

  // write the data to disk using both the memory space and the data space.
  H5Dwrite(dataset1, H5T_IEEE_F64LE, memspace, filespace, xfer_plist,
           &(data1[0][0]));
  //H5Dwrite(dataset2, H5T_IEEE_F64LE, memspace, filespace, xfer_plist,
  //       &(data2[0][0]));

  H5Dclose(dataset1);
  //H5Dclose(dataset2);

  H5Pclose(xfer_plist);

  H5Fclose(file_identifier);
}

hid_t create_hdf5_file(const char *filename, MPI_Comm mpi_hdf5_comm){
  hid_t file_creation_plist = H5P_DEFAULT;   // File creation property list
  // set the file access template for parallel IO access
  hid_t file_access_plist   = H5P_DEFAULT;   // File access property list
  file_access_plist = H5Pcreate(H5P_FILE_ACCESS);

  // set collective mode for metadata writes
  H5Pset_coll_metadata_write(file_access_plist, true);

  MPI_Info mpi_info = MPI_INFO_NULL; // For MPI IO hints
  MPI_Info_create(&mpi_info);
  MPI_Info_set(mpi_info, "striping_factor", "8");
  MPI_Info_set(mpi_info, "striping_unit", "4194304");

  // tell the HDF5 library that we want to use MPI-IO to do the writing
  H5Pset_fapl_mpio(file_access_plist, mpi_hdf5_comm, mpi_info);

  // Open the file collectively
  // H5F_ACC_TRUNC - overwrite existing file. H5F_ACC_EXCL - no overwrite
  // 3rd argument is file creation property list. Using default here
  // 4th argument is the file access property list identifier
  hid_t file_identifier = H5Fcreate(filename, H5F_ACC_TRUNC, file_creation_plist,
                  file_access_plist);

  // release the file access template
  H5Pclose(file_access_plist);
  MPI_Info_free(&mpi_info);

  return file_identifier;
}

hid_t create_hdf5_dataset(hid_t file_identifier, hid_t filespace){
  // create the dataset
  hid_t link_creation_plist    = H5P_DEFAULT; // Link creation property list
  hid_t dataset_creation_plist = H5P_DEFAULT; // Dataset creation property list
  hid_t dataset_access_plist   = H5P_DEFAULT; // Dataset access property list
  hid_t dataset = H5Dcreate2(
    file_identifier,          // Arg 1: file identifier
    "data array",             // Arg 2: dataset name
    H5T_IEEE_F64LE,           // Arg 3: datatype identifier
    filespace,                // Arg 4: filespace identifier
    link_creation_plist,      // Arg 5: link creation property list
    dataset_creation_plist,   // Arg 6: dataset creation property list
    dataset_access_plist);    // Arg 7: dataset access property list

  return dataset;
}

void read_hdf5_file(const char *filename, double **data1, hid_t memspace,
    hid_t filespace, MPI_Comm mpi_hdf5_comm) {
  hid_t file_identifier = open_hdf5_file(filename, mpi_hdf5_comm);

  // Create property list for collective dataset write.
  hid_t xfer_plist = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);

  hid_t dataset1 = open_hdf5_dataset(file_identifier);
  // read the data from disk using both the memory space and the data space.
  H5Dread(dataset1, H5T_IEEE_F64LE, memspace, filespace, H5P_DEFAULT,
          &(data1[0][0]));
  H5Dclose(dataset1);

  H5Pclose(xfer_plist);

  H5Fclose(file_identifier);
}

hid_t open_hdf5_file(const char *filename, MPI_Comm mpi_hdf5_comm){
  // set the file access template for parallel IO access
  hid_t file_access_plist = H5P_DEFAULT;   // File access property list
  file_access_plist = H5Pcreate(H5P_FILE_ACCESS);

  // set collective mode for metadata reads (ops)
  H5Pset_all_coll_metadata_ops(file_access_plist, true);

  MPI_Info mpi_info = MPI_INFO_NULL; // For MPI IO hints
  MPI_Info_create(&mpi_info);
  MPI_Info_set(mpi_info, "striping_factor", "8");
  MPI_Info_set(mpi_info, "striping_unit", "4194304");

  // tell the HDF5 library that we want to use MPI-IO to do the writing
  H5Pset_fapl_mpio(file_access_plist, mpi_hdf5_comm, mpi_info);

  // Open the file collectively
  // H5F_ACC_TRUNC - overwrite existing file. H5F_ACC_EXCL - no overwrite
  // 3rd argument is file creation property list. Using default here
  // 4th argument is the file access property list identifier
  hid_t file_identifier = H5Fopen(filename, H5F_ACC_RDWR, file_access_plist);

  // release the file access template
  H5Pclose(file_access_plist);
  MPI_Info_free(&mpi_info);

  return file_identifier;
}

hid_t open_hdf5_dataset(hid_t file_identifier){
  // open the dataset
  hid_t dataset_access_plist   = H5P_DEFAULT; // Dataset access property list
  hid_t dataset = H5Dopen2(
    file_identifier,        // Arg 1: file identifier
    "data array",           // Arg 2: dataset name to match for read
    dataset_access_plist);  // Arg 3: dataset access property list

  return dataset;
}
