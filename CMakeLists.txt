cmake_minimum_required(VERSION 3.15)
project(ParallelWordCount LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)   # always benchmark optimised code
endif()

find_package(MPI REQUIRED)        # MS-MPI on Windows, Open MPI / MPICH on Linux
find_package(OpenMP REQUIRED)

# Everything except main() is a library, so the tests can link against it.
add_library(wccore STATIC
  src/tokenizer.cpp
  src/file_list.cpp
  src/partition.cpp
  src/file_reader.cpp
  src/wordcount_serial.cpp
  src/wordcount_omp.cpp
  src/histogram.cpp
  src/mpi_comm.cpp
  src/pipeline.cpp
  src/options.cpp
  src/report.cpp
  src/logger.cpp
  src/generator.cpp)
target_include_directories(wccore PUBLIC include)
target_link_libraries(wccore PUBLIC MPI::MPI_CXX OpenMP::OpenMP_CXX)

add_executable(wc src/main.cpp)
target_link_libraries(wc PRIVATE wccore)

add_executable(test_partition tests/test_partition.cpp)
target_link_libraries(test_partition PRIVATE wccore)

enable_testing()
add_test(NAME partition_and_counting COMMAND test_partition)
