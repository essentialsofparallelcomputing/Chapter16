#include <stdio.h>
#include <stdlib.h>

#include "hdf5_file_ops.h"
#include "malloc2D.h"

void init_array(int ny, int nx, int ng, double **array);

int main(int argc, char *argv[])
{
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // for multiple files, subdivide communicator and set colors for each set
  MPI_Comm mpi_hdf5_comm = MPI_COMM_NULL;
  MPI_Comm_dup(MPI_COMM_WORLD, &mpi_hdf5_comm);
  int ncolors = 1, color = 0, nprocs_color, rank_color;
  MPI_Comm_size(mpi_hdf5_comm, &nprocs_color);
  MPI_Comm_rank(mpi_hdf5_comm, &rank_color);

  // set the dimensions of our data array and the number of ghost cells
  int ny = 10, nx = 10, ng = 2, nx_global;
  MPI_Allreduce(&nx, &nx_global, 1, MPI_INT, MPI_SUM, mpi_hdf5_comm);
  int nx_offset[nprocs_color], nx_count[nprocs_color];
  MPI_Allgather(&nx, 1, MPI_INT, nx_count, 1, MPI_INT, mpi_hdf5_comm);
  nx_offset[0] = 0;
  for (int i = 1; i < nprocs_color; i++){
    nx_offset[i] = nx_offset[i-1] +nx_count[i-1];
  }

  double **data1 = (double **)malloc2D(ny+2*ng, nx+2*ng);
  double **data_restore = (double **)malloc2D(ny+2*ng, nx+2*ng);
  init_array(ny, nx, ng, data1);
  for (int j=0; j<ny+2*ng; j++){
    for (int i=0; i<nx+2*ng; i++){
      data_restore[j][i] = 0.0;
    }
  }

  hid_t memspace = H5S_NULL, filespace = H5S_NULL;
  hdf5_file_init(ny, nx, ng, nx_global, nx_offset[rank_color], mpi_hdf5_comm,
                 &memspace, &filespace);

  char filename[30];
  if (ncolors > 1) {
    sprintf(filename,"example_%02d.hdf5",color);
  } else {
    sprintf(filename,"example.hdf5",color);
  }

  // Do the computation and write out a sequence of files
  write_hdf5_file(filename, data1, memspace, filespace, mpi_hdf5_comm);
  // Read back the data for verifying the file operations
  read_hdf5_file(filename, data_restore, memspace, filespace, mpi_hdf5_comm);

  hdf5_file_finalize(&memspace, &filespace);

  if (rank == 0) printf("Verifying  checkpoint\n");

  int ierr = 0;
  // verification
  for (int j=0; j<ny+2*ng; j++){
    for (int i=0; i<nx+2*ng; i++){
      if (data_restore[j][i] != data1[j][i]) {
        ierr++;
        printf("DEBUG -- j %d i %d restored %lf data %lf\n",
               j,i,data_restore[j][i],data1[j][i]);
      }
    }
  }
  int ierr_global = 0;
  MPI_Allreduce(&ierr, &ierr_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  if (rank == 0 && ierr_global == 0) printf("   Checkpoint has been verified\n");

  free(data1);
  free(data_restore);

  MPI_Comm_free(mpi_hdf5_comm);
  MPI_Finalize();
  return 0;
}

void init_array(int ny, int nx, int ng, double **array) {
  // initialize the guardcells to 0, and the interior i+j+100*rank
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for (int j = 0; j < ny+2*ng; j++) {
    for (int i = 0; i < nx+2*ng; i++) {
      array[j][i] = 0.0;
    }
  }

  int icount = 1;
  for (int j = ng; j < ny+ng; j++) {
    for (int i = ng; i < nx+ng; i++) {
      array[j][i] = (double) (icount+100*rank);
      icount++;
    }
  }
}
