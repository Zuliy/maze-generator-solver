#ifndef MAZE_H
#define MAZE_H

#include <chrono>
#include <thread>
#include <stack>
#include <utility>
#include <cstdlib>
#include <vector>
using namespace std;

class Maze
{
private:
    int R, C;

    vector<vector<bool>> northWall;
    vector<vector<bool>> eastWall;
    vector<vector<bool>> visited;
    stack<pair<int, int>> st;

public:
    Maze(int rows, int cols);
    void display();
    void generateMaze();
    void solveMaze();
};

#endif