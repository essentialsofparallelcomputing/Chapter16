#include "hdf5.h"
#include "mpi.h"

void write_hdf5_file(const char *filename, double **data, hid_t memspace, hid_t filespace, MPI_Comm mpi_hdf5_comm);
void read_hdf5_file(const char *filename, double **data, hid_t memspace, hid_t filespace, MPI_Comm mpi_hdf5_comm);
void hdf5_file_init(int ny, int nx, int ng, int nx_global, int nx_offset, MPI_Comm mpi_hdf5_comm, hid_t *memspace, hid_t *filespace);
void hdf5_file_finalize(hid_t *memspace, hid_t *filespace);