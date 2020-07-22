All: MPI_IO HDF5_IO

.PHONY: MPI_IO HDF5_IO

MPI_IO: MPI_IO_Examples/mpi_io_block2d/build/mpi_io_block2d
	cd MPI_IO_Examples/mpi_io_block2d && mkdir build && cd build && cmake .. && make ./mpi_io_block2d

HDF5: HDF5Examples/hdf5block2d/build/hdf5block2d
	cd HDF5Examples/hdf5block2d && mkdir build && cd build && cmake .. && make ./hdf5block2d

clean:
	cd MPI_IO_Examples/mpi_io_block2d && rm -rf build
	cd HDF5Examples/hdf5block2d && rm -rf build

