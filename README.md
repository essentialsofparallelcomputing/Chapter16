# Chapter16 File operations for a parallel world
This is from Chapter 16 of Parallel and High Performance Computing book, Robey and Zamora
Manning Publications, available at http://manning.com

The book may be obtained at
   http://www.manning.com/?a_aid=ParallelComputingRobey

Copyright 2019-2020 Robert Robey, Yuliana Zamora, and Manning Publications
Emails: brobey@earthlink.net, yzamora215@gmail.com

See License.txt for licensing information.

MPI-IO and HDF5 requires either OpenMPI or MPICH

# MPI-IO example (Book: listings 16.1 through 16.4)
      In MPI_IO_Examples/mpi_io_block2d directory
      Uses a cmake build system
      mkdir build && cd build && cmake .. && make
      Run example with mpirun -n 4 ./mpi_io_block2d

# HDF5 example (Book: listings 16.5 through 16.8)
      In HDF5Examples/hdf5block2d
      mkdir build && cd build && cmake .. && make
      mpirun -n 4 ./hdf5block2d
      h5dump -y example.hdf5

# MPI-IO Hardware and software interface
      ompi_info |grep "MCA io:"
      ompi_info | --param io ompio --level 9 |grep ":parameter"
      mpirun --mca -io_ompio_verbose_info_parsing 1 -n 4 ./mpi_io_block2d
      export ROMIO_PRINT_HINTS=1; mpirun -n 4 ./mpi_io_block2d
      export MPICH_MPIIO_HINTS_DISPLAY=1; srun -n 4 ./mpi_io_block2d
      export MPICH_MPIIO_STATS=1; srun -n 4 ./mpi_io_block2d

# File system detection program
      In MPI_IO_Examples/mpi_io_block2d directory
      uses cmake build system
      mkdir build && cd build && cmake .. && make
      grep `./fs_detect |cut -f 4 -d' '` /usr/include/linux/magic.h ../fs_detect.c
