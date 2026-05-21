#include <iostream>
#include "../include/Maze.h"

using namespace std;

int main()
{
    Maze maze(10, 10);

    maze.generateMaze();

    maze.display();

    return 0;
}