 // Headers
#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <stack>
#include <thread>
#include <algorithm>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <sstream>
#include <cmath>

#include "gpuErrchk.h"

#define N_MAX 10 //max N board size

#define Threads 512 // threads allocation, hardware dependant (512 is the highest my hardware could go before failing)

// Struct to store result and total time taken
struct Result {
    int totalSolutions;
    std::chrono::duration<double> elapsedTime;
};

__device__ bool boardIsValidSoFar(int lastPlacedRow, const int* gameBoard, const int N)
{
    int lastPlacedColumn = gameBoard[lastPlacedRow];

    for (int row = 0; row < lastPlacedRow; ++row)
    {
        if (gameBoard[row] == lastPlacedColumn)
            return false;
        const auto col1 = lastPlacedColumn - (lastPlacedRow - row);
        const auto col2 = lastPlacedColumn + (lastPlacedRow - row);
        if (gameBoard[row] == col1 || gameBoard[row] == col2)
            return false;
    }
    return true;
}

__global__ void boardPossibilites(int N, long long int maxPossible, long long int offset, int* d_allSolutions, int* d_totalSolutions) {
    // Calculate the column index for the current thread
    long long int column = (long long int)(threadIdx.x + blockIdx.x * blockDim.x) + offset;
    if (column >= maxPossible)
        return;
    // Create an array to represent the game board
    int gameBoard[N_MAX];
    for (int i = 0; i < N; i++) {
        // Extract the column value for each row from the column index
        gameBoard[i] = column % N;
        // Check if the current placement is valid so far
        if (!boardIsValidSoFar(i, gameBoard, N))
            return;
        // Divide the column index by N to move to the next row
        column /= N;
    }
    // Increment the total solutions counter and get the new index
    const int index = atomicAdd(d_totalSolutions, 1);
    // Store the game board configuration for the current solution
    for (int i = 0; i < N; i++)
        d_allSolutions[N * index + i] = gameBoard[i];
}

Result systemStart(int N) {
    // Create a Result struct to store results
    Result result{}; 
    // Initialisation of total results amount
    result.totalSolutions = 0; 
    // Calculate the max number of possibilites 
    const long long int maxPossible = std::powl(N, N); 
    // Calculate the size of memory needed for solutions, N^5 to make sure there's any memory to store all solutions for higher N without getting a performance hit, 5 is highest it goes before slowdown on current hardware
    size_t solutionsMemory = std::powl(N, 5) * sizeof(int*); 
    // Pointer for the device memory to store all solutions and total solutions number
    int* d_allSolutions = 0; 
    int* d_totalSolutions = 0; 
    // Allocate device memory for all solutions and total solutions number
    cudaMalloc((void**)&d_allSolutions, solutionsMemory);
    cudaMalloc((void**)&d_totalSolutions, sizeof(int)); 
    // Number of kernel executions, set to one so that it runs
    int kernerExec = 1; 
    // Grid size for block execution set to double of thread size
    int grid = Threads * 2;
    // Block size for thread execution set to size of thread
    int block = Threads;
    // Check if the maximum possible number of combinations is greater than the grid * block size
    if (maxPossible > grid * block) 
        // Calculate the number of kernel executions needed
        kernerExec = std::ceil(static_cast<double>(maxPossible) / (grid * block)); 
    // Iterate over the number of kernel executions
    for (long long int i = 0; i < kernerExec; i++) { 
        // Launch the kernel with specified grid and block sizes
        boardPossibilites << <grid, block >> > (N, maxPossible, static_cast<long long int>(grid * block * i), d_allSolutions, d_totalSolutions); 
        // Synchronize device threads after kernel execution
        cudaDeviceSynchronize(); 
    }
    // Copy the total number of solutions from device to host
    cudaMemcpy(&result.totalSolutions, d_totalSolutions, sizeof(int), cudaMemcpyDeviceToHost); 
    // Free device memory for the total number of solutions
    cudaFree(d_totalSolutions);
    // Return the Result struct containing the total number of solutions
    return result; 
}

void calculateAllSolutions(int N) {
    Result result;
    auto start = std::chrono::system_clock::now();
    result = systemStart(N);
    auto stop = std::chrono::system_clock::now();

    result.elapsedTime = stop - start;

    printf("N=%d, solutions=%d\n", N, result.totalSolutions);
    printf("Time to find solutions for N=%d: %.0f microseconds\n", N, std::chrono::duration<double, std::micro>(result.elapsedTime).count());

}

int main(int argc, char** argv)
{
    gpuErrchk(cudaSetDevice(0));

    for (int N = 4; N <= N_MAX; ++N)
        calculateAllSolutions(N);

    return 0;
}