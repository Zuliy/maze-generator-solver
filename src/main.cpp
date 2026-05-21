#include <iostream>
#include <ctime>
#include "../include/Maze.h"

using namespace std;

int main()
{
    srand(time(0)); // important for randomness

    Maze maze(10, 10);

    maze.generateMaze();

    maze.solveMaze();

    return 0;
}