cmake_minimum_required(VERSION 3.0)
project(hdf5block2d)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_policy(SET CMP0060 NEW) ## use full path so it doesn't pick up hdf5 in /usr/lib
set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Require MPI for this project:
find_package(MPI REQUIRED)

find_package(HDF5 1.10.6 REQUIRED)
if (HDF5_IS_PARALLEL)
    message("found parallel HDF5")
endif (HDF5_IS_PARALLEL)


find_package(Vector)
if (CMAKE_VECTOR_VERBOSE)
    set(VECTOR_C_FLAGS "${VECTOR_C_FLAGS} ${VECTOR_C_VERBOSE}")
    set(VECTOR_CXX_FLAGS "${VECTOR_CXX_FLAGS} ${VECTOR_CXX_VERBOSE}")
endif (CMAKE_VECTOR_VERBOSE)
if (CMAKE_VECTOR_NOVEC)
    set(VECTOR_C_FLAGS "${VECTOR_NOVEC_C_FLAGS}")
    set(VECTOR_CXX_FLAGS "${VECTOR_NOVEC_CXX_FLAGS}")
endif (CMAKE_VECTOR_NOVEC)

add_executable(hdf5block2d hdf5_file_ops.c hdf5block2d.c malloc2D.c)

set (IncludeDirs "${MPI_CXX_INCLUDE_PATH};${HDF5_C_INCLUDE_DIRS}")
set_target_properties(hdf5block2d PROPERTIES INCLUDE_DIRECTORIES "${IncludeDirs}")
set_target_properties(hdf5block2d PROPERTIES COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
set_target_properties(hdf5block2d PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
target_link_libraries(hdf5block2d ${HDF5_C_LIBRARIES})
target_link_libraries(hdf5block2d ${MPI_CXX_LIBRARIES})

# Cleanup
add_custom_target(distclean COMMAND rm -rf CMakeCache.txt CMakeFiles
                  Makefile cmake_install.cmake *.hdf5)
