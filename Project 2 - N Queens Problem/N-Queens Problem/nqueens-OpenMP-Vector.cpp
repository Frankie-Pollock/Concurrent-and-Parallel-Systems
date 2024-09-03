 // Headers
#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <thread>

using namespace std;

// Check if the chessboard is valid so far, for row in [0, lastPlacedRow]
bool boardIsValidSoFar(int lastPlacedRow, const vector<int>& gameBoard, int N)
{
    int lastPlacedColumn = gameBoard[lastPlacedRow];

    // Check against other queens
    for (int row = 0; row < lastPlacedRow; ++row)
    {
        if (gameBoard[row] == lastPlacedColumn) // same column, fail!
            return false;
        // Check the 2 diagonals
        const auto col1 = lastPlacedColumn - (lastPlacedRow - row);
        const auto col2 = lastPlacedColumn + (lastPlacedRow - row);
        if (gameBoard[row] == col1 || gameBoard[row] == col2)
            return false;
    }
    return true;
}

// Calculate all solutions given the size of the chessboard
void calculateAllSolutions(int N, bool print)
{
    vector<vector<int>> solutions;
    vector<int> gameBoard(N);
    vector<int> column(N, -1);

    int row = 0;

    auto start = chrono::steady_clock::now(); // Start the timer

    while (row >= 0)
    {
        // Increment the column in the current row
        column[row]++;
        // If the column exceeds the board size, backtrack to the previous row
        if (column[row] >= N)
        {
            row--;
            continue;
        }
        // Set the column for the current row
        gameBoard[row] = column[row];
        // If the board is valid so far
        if (boardIsValidSoFar(row, gameBoard, N))
        {
            // If the chessboard is fully filled, save the solution
            if (row == N - 1)
            {
                solutions.push_back(gameBoard);
            }
            else
            {
                // Move to the next row
                row++;
                column[row] = -1;
            }
        }
    }
    auto duration = chrono::steady_clock::now() - start; // Calculate the duration
    int solutionsCount = solutions.size();
    printf("N=%d, solutions=%d\n", N, solutionsCount);
    printf("Time to find solutions for N=%d: %.0f microseconds\n", N, std::chrono::duration<double, std::micro>(duration).count());
}


int main(int argc, char** argv)
{
    auto num_threads = thread::hardware_concurrency();
    // Enable OpenMP parallelization on the outer loop
#pragma omp parallel for num_threads(num_threads)
    for (int N = 4; N < 11; ++N)
        calculateAllSolutions(N, false);
}
