#include <iostream>
#include <cstdlib>
#include <ctime>
#include "../include/Maze.h"

using namespace std;

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    // Maze dimensions — change these to make larger mazes
    const int ROWS = 10;
    const int COLS = 10;

    // Start at bottom-left, end at top-right
    // (row 0 is the bottom due to the phantom-row convention)
    const int START_R = 0, START_C = 0;
    const int END_R = ROWS - 1, END_C = COLS - 1;

    cout << "=== Maze Generator & Solver ===\n"
         << "  Grid : " << ROWS << " x " << COLS << "\n"
         << "  Start: (" << START_R << "," << START_C << ")  "
         << "  End  : (" << END_R << "," << END_C << ")\n"
         << "\nPress Enter to begin generation...\n";
    cin.get();

    Maze maze(ROWS, COLS, START_R, START_C, END_R, END_C);

    maze.generateMaze(); // watch the mouse carve the maze
    maze.solveMaze();    // watch the backtracking solver find the path

    return 0;
}