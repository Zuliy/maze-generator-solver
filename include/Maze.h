#ifndef MAZE_H
#define MAZE_H

#include <vector>
using namespace std;

class Maze
{
private:
    int R, C;

    vector<vector<bool>> northWall;
    vector<vector<bool>> eastWall;
    vector<vector<bool>> visited;

public:
    Maze(int rows, int cols);
    void display();
};

#endif