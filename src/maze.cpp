#include "../include/Maze.h"
#include <iostream>

using namespace std;

Maze::Maze(int rows, int cols)
{
    R = rows;
    C = cols;

    northWall = vector<vector<bool>>(R, vector<bool>(C, true));
    eastWall = vector<vector<bool>>(R, vector<bool>(C, true));
    visited = vector<vector<bool>>(R, vector<bool>(C, false));
}

void Maze::display()
{
    // TOP BORDER
    for (int j = 0; j < C; j++)
    {
        cout << "+---";
    }
    cout << "+" << endl;

    for (int i = 0; i < R; i++)
    {

        // LEFT WALL
        cout << "|";

        for (int j = 0; j < C; j++)
        {
            cout << "   ";

            if (eastWall[i][j])
                cout << "|";
            else
                cout << " ";
        }
        cout << endl;

        // BOTTOM WALLS
        for (int j = 0; j < C; j++)
        {
            if (northWall[i][j])
                cout << "+---";
            else
                cout << "+   ";
        }
        cout << "+" << endl;
    }
}