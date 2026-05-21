#include "../include/Maze.h"
#include <iostream>
#include <chrono>
#include <thread>

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

void Maze::generateMaze()
{

    // start from (0,0)
    int r = 0;
    int c = 0;

    visited[r][c] = true;
    st.push({r, c});

    while (!st.empty())
    {

        r = st.top().first;
        c = st.top().second;

        vector<pair<int, int>> neighbors;

        // UP
        if (r > 0 && !visited[r - 1][c])
            neighbors.push_back({r - 1, c});

        // DOWN
        if (r < R - 1 && !visited[r + 1][c])
            neighbors.push_back({r + 1, c});

        // LEFT
        if (c > 0 && !visited[r][c - 1])
            neighbors.push_back({r, c - 1});

        // RIGHT
        if (c < C - 1 && !visited[r][c + 1])
            neighbors.push_back({r, c + 1});

        if (!neighbors.empty())
        {

            // pick random neighbor
            int idx = rand() % neighbors.size();
            int nr = neighbors[idx].first;
            int nc = neighbors[idx].second;

            // REMOVE WALL between (r,c) and (nr,nc)

            // vertical move
            if (nr == r + 1)
                northWall[r][c] = false;
            if (nr == r - 1)
                northWall[nr][nc] = false;

            // horizontal move
            if (nc == c + 1)
                eastWall[r][c] = false;
            if (nc == c - 1)
                eastWall[nr][nc] = false;

            visited[nr][nc] = true;
            st.push({nr, nc});
        }
        else
        {
            st.pop(); // backtrack
        }
    }
}

void delay()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Maze::solveMaze()
{

    stack<pair<int, int>> st;
    vector<vector<bool>> visitedSolve(R, vector<bool>(C, false));

    int r = 0, c = 0;
    st.push({r, c});
    visitedSolve[r][c] = true;

    while (!st.empty())
    {

        r = st.top().first;
        c = st.top().second;

        // MARK CURRENT PATH
        cout << "\033[2J\033[1;1H"; // clear screen (IMPORTANT)

        for (int i = 0; i < R; i++)
        {
            for (int j = 0; j < C; j++)
            {

                if (i == r && j == c)
                    cout << "R "; // red mouse

                else if (visitedSolve[i][j])
                    cout << ". "; // visited path

                else
                    cout << "# "; // unexplored
            }
            cout << endl;
        }

        delay();

        // GOAL
        if (r == R - 1 && c == C - 1)
        {
            cout << "Maze Solved!" << endl;
            return;
        }

        vector<pair<int, int>> neighbors;

        if (r > 0 && !visitedSolve[r - 1][c] && !northWall[r - 1][c])
            neighbors.push_back({r - 1, c});

        if (r < R - 1 && !visitedSolve[r + 1][c] && !northWall[r][c])
            neighbors.push_back({r + 1, c});

        if (c > 0 && !visitedSolve[r][c - 1] && !eastWall[r][c - 1])
            neighbors.push_back({r, c - 1});

        if (c < C - 1 && !visitedSolve[r][c + 1] && !eastWall[r][c])
            neighbors.push_back({r, c + 1});

        if (!neighbors.empty())
        {

            int idx = rand() % neighbors.size();
            int nr = neighbors[idx].first;
            int nc = neighbors[idx].second;

            visitedSolve[nr][nc] = true;
            st.push({nr, nc});
        }
        else
        {
            st.pop();
        }
    }
}
