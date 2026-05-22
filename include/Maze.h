#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <stack>
#include <string>
#include <utility>

using namespace std;

// ============================================================
//  Maze — wall-based representation
//
//  northWall[i][j] = true  →  top edge of cell (i,j) is solid
//  eastWall [i][j] = true  →  right edge of cell (i,j) is solid
//
//  PHANTOM ROW CONVENTION:
//    northWall[0][j] values form the BOTTOM boundary of the maze.
//    Rows are displayed R-1 first, 0 last so row 0 produces the
//    bottom edge — the "phantom row" described in the assignment.
//
//  SMOOTH ANIMATION STRATEGY:
//    drawFull() is called ONCE per phase to paint the entire grid.
//    After that, display() uses ANSI cursor-move codes to overwrite
//    only the 3 characters of the cell that moved, and only the
//    one wall segment that was just eaten.  The rest of the grid
//    is never touched, so there is zero flicker.
//
//  Cell states (solver):
//    PATH     — on the active path   (red  '.')
//    DEAD     — backtracked dead end (blue 'B')
//    UNVISITED — not reached yet     ('   ')
// ============================================================

class Maze
{
public:
    Maze(int rows, int cols, int sr, int sc, int er, int ec);

    void generateMaze(); // DFS "mouse eats walls", animated
    void solveMaze();    // Backtracking solver, animated

private:
    // ---- Dimensions ----
    int R, C;

    // ---- Start / end ----
    int startR, startC;
    int endR, endC;

    // ---- Entry/exit gap rows ----
    int entryRow;
    int exitRow;

    // ---- Wall arrays ----
    vector<vector<bool>> northWall; // northWall[row][col] = top wall intact
    vector<vector<bool>> eastWall;  // eastWall [row][col] = right wall intact

    // ---- Generation state ----
    vector<vector<bool>> visited;
    stack<pair<int, int>> genStack;

    // ---- Solver cell state ----
    enum CellState
    {
        UNVISITED,
        PATH,
        DEAD
    };
    vector<vector<CellState>> cellState;

    // ---- Smooth-animation state ----
    int prevMouseR, prevMouseC; // where R was drawn last frame
    string statusText;          // shown below the maze

    // ---- Rendering helpers ----
    void drawFull(int mouseR, int mouseC); // full initial paint
    void display(int mouseR, int mouseC);  // diff-only update
    void printCell(int i, int j, int mouseR, int mouseC);
    void updateCell(int i, int j, int mouseR, int mouseC);
    void updateNorthWall(int i, int j);
    void updateEastWall(int i, int j);

    // ---- Algorithm helpers ----
    void delay(int ms);
    vector<pair<int, int>> unvisitedNeighbours(int r, int c) const;
    vector<pair<int, int>> openNeighbours(int r, int c,
                                          const vector<vector<bool>> &seen) const;
    void openEntryExit();
    void eatExtraWall();
};

#endif // MAZE_H