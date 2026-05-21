#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <stack>
#include <utility>
using namespace std;

class Maze
{
private:
    int R, C;

    vector<vector<bool>> northWall;
    vector<vector<bool>> eastWall;
    vector<vector<bool>> visited;
    stack<pair<int, int>> st;

    void delay();

public:
    Maze(int rows, int cols);

    void display(int cr, int cc);
    void generateMaze();
    void solveMaze();
};

#endif