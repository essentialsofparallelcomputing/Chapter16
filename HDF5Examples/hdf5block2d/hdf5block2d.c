#include <stdio.h>
#include <stdlib.h>

#include "hdf5_file_ops.h"
#include "malloc2D.h"

void init_array(int ny, int nx, int ng, double **array);

int main(int argc, char *argv[])
{
  MPI_Init(&argc, &argv);

  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  // for multiple files, subdivide communicator and set colors for each set
  MPI_Comm mpi_hdf5_comm = MPI_COMM_NULL;
  int nfiles = 1;
  float ranks_per_file = (float)nprocs/(float)nfiles;
  int color = (int)((float)rank/ranks_per_file);
  MPI_Comm_split(MPI_COMM_WORLD, color, rank, &mpi_io_comm);
  int nprocs_color, rank_color;
  MPI_Comm_size(mpi_hdf5_comm, &nprocs_color);
  MPI_Comm_rank(mpi_hdf5_comm, &rank_color);
  int row_color = 1, col_color = rank_color;
  MPI_Comm mpi_row_comm, mpi_col_comm;
  MPI_Comm_split(mpi_hdf5_comm, row_color, rank_color, &mpi_row_comm);
  MPI_Comm_split(mpi_hdf5_comm, col_color, rank_color, &mpi_col_comm);

  // set the dimensions of our data array and the number of ghost cells
  int ndims = 2, ng = 2, ny = 10, nx = 10;
  int global_subsizes[] = {ny, nx};

  int ny_offset = 0, nx_offset = 0;
  MPI_Exscan(&nx, &nx_offset, 1, MPI_INT, MPI_SUM, mpi_row_comm);
  MPI_Exscan(&ny, &ny_offset, 1, MPI_INT, MPI_SUM, mpi_col_comm);

  int ny_global, nx_global;
  MPI_Allreduce(&nx, &nx_global, 1, MPI_INT, MPI_SUM, mpi_row_comm);
  MPI_Allreduce(&ny, &ny_global, 1, MPI_INT, MPI_SUM, mpi_col_comm);

  double **data = (double **)malloc2D(ny+2*ng, nx+2*ng);
  double **data_restore = (double **)malloc2D(ny+2*ng, nx+2*ng);
  init_array(ny, nx, ng, data);
  for (int j=0; j<ny+2*ng; j++){
    for (int i=0; i<nx+2*ng; i++){
      data_restore[j][i] = 0.0;
    }
  }

  hid_t memspace = H5S_NULL, filespace = H5S_NULL;
  hdf5_file_init(ng, ny_global, nx_global, ny, nx, ny_offset, nx_offset,
                 mpi_hdf5_comm, &memspace, &filespace);

  char filename[30];
  if (ncolors > 1) {
    sprintf(filename,"example_%02d.hdf5",color);
  } else {
    sprintf(filename,"example.hdf5");
  }

  // Do the computation and write out a sequence of files
  write_hdf5_file(filename, data, memspace, filespace, mpi_hdf5_comm);
  // Read back the data for verifying the file operations
  read_hdf5_file(filename, data_restore, memspace, filespace, mpi_hdf5_comm);

  hdf5_file_finalize(&memspace, &filespace);

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
  
  if (rank == 0) {
     hsize_t dims[] = {ny, nx};
     filespace = H5Screate_simple(ndims, dims, NULL);
     hsize_t  start[] = {0,         0};
     hsize_t stride[] = {1,         1}; 
     hsize_t  count[] = {ny_global, nx_global};

     H5Sselect_hyperslab(filespace, H5S_SELECT_SET,
                start, stride, count, NULL);

     dims[0] = ny_global+2*ng;
     dims[1] = nx_global+2*ng;
     memspace = H5Screate_simple(ndims, dims, NULL);
     hsize_t  mstart[] = {ng,   ng};
     hsize_t mstride[] = {1,    1}; 
     hsize_t  mcount[] = {ny_global,   nx_global};

     H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
                      mstart, mstride, mcount, NULL);

     hid_t file_identifier = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
     hid_t dataset = H5Dopen2(file_identifier, "data array", H5P_DEFAULT);

     double **data_check = (double **)malloc2D(ny_global+2*ng, nx_global+2*ng);
     H5Dread(dataset, H5T_IEEE_F64LE, memspace, filespace, H5P_DEFAULT,
          &(data_check[0][0]));
     /*

     for (int j = 0; j < ny_global; j++){
       for (int i = 0; i < nx_global; i++){
         printf("x[%d][%d] %lf\n",j,i,data_check[j+ng][i+ng]);
       }
     }   
     */
  }

  free(data);
  free(data_restore);

  MPI_Comm_free(&mpi_hdf5_comm);
  MPI_Comm_free(&mpi_row_comm);
  MPI_Comm_free(&mpi_col_comm);

  MPI_Finalize();
  return 0;
}

void init_array(int ny, int nx, int ng, double **array) {
  // initialize the halo cells to 0, and the interior i+j+100*rank
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
