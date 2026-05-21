#include <iostream>
#include "../include/Maze.h"

using namespace std;

int main()
{
    Maze maze(10, 10);

    maze.generateMaze();

    maze.solveMaze();

    return 0;
}