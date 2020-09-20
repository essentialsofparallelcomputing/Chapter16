All: MPI_IO HDF5 Hardware_Software_info File_Query

.PHONY: MPI_IO HDF5 Hardware_Software_info File_Query

MPI_IO: MPI_IO_Examples/mpi_io_block2d/build/mpi_io_block2d

MPI_IO_Examples/mpi_io_block2d/build/mpi_io_block2d:
	cd MPI_IO_Examples/mpi_io_block2d && mkdir build && cd build && cmake .. && make && \
	mpirun -n 4 ./mpi_io_block2d

HDF5: HDF5Examples/hdf5block2d/build/hdf5block2d

HDF5Examples/hdf5block2d/build/hdf5block2d:
	cd HDF5Examples/hdf5block2d && mkdir build && cd build && cmake .. && make && \
	mpirun -n 4 ./hdf5block2d

Hardware_Software_info:
	ompi_info |grep "MCA io:"
	ompi_info --param io ompio --level 9 |grep ": parameter"
	cd MPI_IO_Examples/mpi_io_block2d && mkdir build_info && cd build_info && cmake .. && make && \
	mpirun --mca -io_ompio_verbose_info_parsing 1 -n 4 ./mpi_io_block2d && \
	export ROMIO_PRINT_HINTS=1; mpirun -n 4 ./mpi_io_block2d
	# MPICH commands
	#export MPICH_MPIIO_HINTS_DISPLAY=1; srun -n 4 ./mpi_io_block2d
	#export MPICH_MPIIO_STATS=1; srun -n 4 ./mpi_io_block2d

File_Query:
	cd MPI_IO_Examples/mpi_io_block2d && \
	mkdir build_query && cd build_query && cmake .. && make && \
	grep `./fs_detect |cut -f 4 -d' '` /usr/include/linux/magic.h ../fs_detect.c

clean:
	cd MPI_IO_Examples/mpi_io_block2d && rm -rf build build_info build_query
	cd HDF5Examples/hdf5block2d && rm -rf build
