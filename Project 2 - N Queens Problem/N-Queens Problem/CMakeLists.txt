project(set10108-cw02 LANGUAGES CXX CUDA)
cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)

#### OPEN MP PACKAGES ####
find_package(OpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")


add_executable(NQueens nqueens.cpp)
add_executable(NQueens-OpenMP-Vector nqueens-OpenMP-Vector.cpp)
add_executable(NQueens-OpenMP-Final nqueens-OpenMP-Final.cpp)
add_executable(NQueens-cuda nqueens-cuda.cu)
set_property(TARGET NQueens-cuda PROPERTY CUDA_SEPARABLE_COMPILATION ON)