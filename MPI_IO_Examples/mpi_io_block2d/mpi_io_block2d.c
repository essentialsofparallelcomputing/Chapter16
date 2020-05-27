#include <stdio.h>
#include <stdlib.h>

#include "mpi_io_file_ops.h"
#include "malloc2D.h"

void init_array(int ny, int nx, int ng, double **array);

int main(int argc, char *argv[])
{
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // for multiple files, subdivide communicator and set colors for each set
  MPI_Comm mpi_io_comm = MPI_COMM_NULL;
  MPI_Comm_dup(MPI_COMM_WORLD, &mpi_io_comm);
  int ncolors = 1, color = 0, nprocs_color, rank_color;
  MPI_Comm_size(mpi_io_comm, &nprocs_color);
  MPI_Comm_rank(mpi_io_comm, &rank_color);

  // set the dimensions of our data array and the number of ghost cells
  int ndim = 2, ng = 2, ny = 10, nx = 10;
  int global_subsizes[] = {ny, nx};

  int ny_offset = 0, nx_offset = 0;
  MPI_Exscan(&nx, &nx_offset, 1, MPI_INT, MPI_SUM, mpi_io_comm);
  int global_offsets[] = {ny_offset, nx_offset};

  int ny_global = ny, nx_global;
  MPI_Allreduce(&nx, &nx_global, 1, MPI_INT, MPI_SUM, mpi_io_comm);
  int global_sizes[ny_global, nx_global];
  int data_size = global_sizes[0]*global_sizes[1];

  double **data = (double **)malloc2D(ny+2*ng, nx+2*ng);
  double **data_restore = (double **)malloc2D(ny+2*ng, nx+2*ng);
  init_array(ny, nx, ng, data);
  for (int j=0; j<ny+2*ng; j++){
    for (int i=0; i<nx+2*ng; i++){
      data_restore[j][i] = 0.0;
    }
  }

  MPI_Datatype memspace = MPI_DATATYPE_NULL, filespace = MPI_DATATYPE_NULL;
  mpi_io_file_init(ng, global_sizes, global_subsizes, global_offsets,
      &memspace, &filespace);

  char filename[30];
  if (ncolors > 1) {
    sprintf(filename,"example_%02d.hdf5",color);
  } else {
    sprintf(filename,"example.hdf5",color);
  }

  // Do the computation and write out a sequence of files
  write_mpi_io_file(filename, data, data_size, memspace, filespace, mpi_io_comm);
  // Read back the data for verifying the file operations
  read_mpi_io_file(filename, data_restore, data_size, memspace, filespace, mpi_io_comm);

  mpi_io_file_finalize(&memspace, &filespace);

  if (rank == 0) printf("Verifying  checkpoint\n");

  int ierr = 0;
  // verification
  for (int j=0; j<ny+2*ng; j++){
    for (int i=0; i<nx+2*ng; i++){
      if (data_restore[j][i] != data[j][i]) {
        ierr++;
        printf("DEBUG -- j %d i %d restored %lf data %lf\n",
               j,i,data_restore[j][i],data[j][i]);
      }
    }
  }
  int ierr_global = 0;
  MPI_Allreduce(&ierr, &ierr_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  if (rank == 0 && ierr_global == 0) printf("   Checkpoint has been verified\n");

  free(data);
  free(data_restore);

  //MPI_Comm_free(mpi_hdf5_comm);
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
