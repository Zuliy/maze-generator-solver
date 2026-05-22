#include "../include/Maze.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <sstream>

using namespace std;

// ============================================================
//  ANSI HELPERS
//  We NEVER call clearScreen() after init. Instead we jump the
//  cursor to a precise terminal (row,col) and overwrite only
//  the characters that actually changed. This eliminates all
//  flicker — the grid lines stay perfectly still and only the
//  moving dot updates.
//
//  Terminal coordinate system (1-based):
//    moveTo(1,1) = top-left corner
//
//  Maze-cell → terminal coordinate mapping:
//    Each maze row i (displayed top-to-bottom, so i = R-1 first)
//    occupies two terminal lines:
//      - the cell interior line
//      - the horizontal wall line below it
//    Plus one top-border line at terminal row 1.
//
//    Terminal row for the interior of maze row i:
//      term_row = 1 + (R - 1 - i) * 2 + 1   =  2 + 2*(R-1-i)
//    Terminal row for the wall line BELOW maze row i:
//      term_row = 2 + 2*(R-1-i) + 1           =  3 + 2*(R-1-i)
//
//    Terminal col for cell interior of maze col j:
//      The left '|' occupies col 1.
//      Each cell is 4 chars wide ("   |" or "   ").
//      Cell interior starts at col 2 + j*4 and is 3 chars wide.
// ============================================================

static const char *RED = "\033[31m";
static const char *GREEN = "\033[32m";
static const char *BLUE = "\033[34m";
static const char *YELLOW = "\033[33m"; // mouse head during generation
static const char *BOLD = "\033[1m";
static const char *RESET = "\033[0m";
static const char *HIDE_CURSOR = "\033[?25l";
static const char *SHOW_CURSOR = "\033[?25h";

// Move terminal cursor to (termRow, termCol) — both 1-based
static void moveTo(int termRow, int termCol)
{
    cout << "\033[" << termRow << ";" << termCol << "H";
}

// Convert maze (row i) → terminal row of the cell-interior line
static int cellTermRow(int i, int R)
{
    return 2 + 2 * (R - 1 - i);
}

// Convert maze col j → terminal col of the LEFT char of cell interior
static int cellTermCol(int j)
{
    return 2 + j * 4; // "| " prefix + j full cells of width 4
}

// ============================================================
//  PORTABLE SLEEP
// ============================================================
void Maze::delay(int ms)
{
    this_thread::sleep_for(chrono::milliseconds(ms));
}

// ============================================================
//  CONSTRUCTOR
// ============================================================
Maze::Maze(int rows, int cols, int sr, int sc, int er, int ec)
    : R(rows), C(cols),
      startR(sr), startC(sc),
      endR(er), endC(ec),
      entryRow(-1), exitRow(-1), // -1 = no gap yet; opened after generation
      northWall(rows, vector<bool>(cols, true)),
      eastWall(rows, vector<bool>(cols, true)),
      visited(rows, vector<bool>(cols, false)),
      cellState(rows, vector<CellState>(cols, UNVISITED)),
      prevMouseR(-1), prevMouseC(-1)
{
}

// ============================================================
//  FULL INITIAL DRAW — called ONCE at the start of each phase.
//  After this, only diffs are written.
// ============================================================
void Maze::drawFull(int mouseR, int mouseC)
{
    // Clear screen and move to top-left
    cout << "\033[2J\033[H";

    // ---- top border ----
    for (int j = 0; j < C; j++)
        cout << "+---";
    cout << "+\n";

    // ---- rows top (R-1) → bottom (0) ----
    for (int i = R - 1; i >= 0; --i)
    {
        // left wall
        cout << (i == entryRow ? " " : "|");

        for (int j = 0; j < C; j++)
        {
            // cell interior
            printCell(i, j, mouseR, mouseC);
            // east wall
            if (j == C - 1 && i == exitRow)
                cout << " ";
            else
                cout << (eastWall[i][j] ? "|" : " ");
        }
        cout << "\n";

        // horizontal wall line below row i
        for (int j = 0; j < C; j++)
            cout << (northWall[i][j] ? "+---" : "+   ");
        cout << "+\n";
    }

    // ---- status line (2 lines below maze) ----
    int statusLine = 2 + 2 * R + 1;
    moveTo(statusLine, 1);
    cout << "  " << RED << "R" << RESET << " = mouse  "
         << GREEN << "G" << RESET << " = goal  "
         << RED << "." << RESET << " = path  "
         << BLUE << "B" << RESET << " = dead end"
         << "          "; // pad to erase leftover text

    cout.flush();
    prevMouseR = mouseR;
    prevMouseC = mouseC;
}

// ============================================================
//  PRINT A SINGLE CELL INTERIOR (3 chars, no trailing wall)
//  Used by both drawFull and updateCell.
// ============================================================
void Maze::printCell(int i, int j, int mouseR, int mouseC)
{
    if (i == mouseR && j == mouseC)
        cout << YELLOW << BOLD << " R " << RESET;
    else if (i == endR && j == endC)
        cout << GREEN << BOLD << " G " << RESET;
    else if (cellState[i][j] == DEAD)
        cout << BLUE << " B " << RESET;
    else if (cellState[i][j] == PATH)
        cout << RED << " . " << RESET;
    else if (visited[i][j])
        cout << "   "; // visited but no special state
    else
        cout << "   "; // unvisited
}

// ============================================================
//  UPDATE ONE CELL IN-PLACE — no flicker, no full redraw.
//  Jumps cursor to exact terminal position and overwrites 3 chars.
// ============================================================
void Maze::updateCell(int i, int j, int mouseR, int mouseC)
{
    int tr = cellTermRow(i, R);
    int tc = cellTermCol(j);
    moveTo(tr, tc);
    printCell(i, j, mouseR, mouseC);
    cout.flush();
}

// ============================================================
//  UPDATE ONE HORIZONTAL WALL SEGMENT (the "---" or "   " part)
//  called after a north wall is eaten between two cells.
//  wallRow i means: the wall line printed BELOW maze row i.
//  Terminal row = cellTermRow(i, R) + 1
// ============================================================
void Maze::updateNorthWall(int i, int j)
{
    int tr = cellTermRow(i, R) + 1; // wall line is one below the cell
    int tc = 1 + j * 4 + 1;         // skip '+', land on the "---"
    moveTo(tr, tc);
    cout << (northWall[i][j] ? "---" : "   ");
    cout.flush();
}

// ============================================================
//  UPDATE ONE VERTICAL WALL SEGMENT (the "|" or " " between cells)
//  called after an east wall is eaten.
//  Terminal position: same row as the cell, after the 3-char interior.
// ============================================================
void Maze::updateEastWall(int i, int j)
{
    int tr = cellTermRow(i, R);
    int tc = cellTermCol(j) + 3; // 3 chars of cell interior, then wall
    moveTo(tr, tc);
    cout << (eastWall[i][j] ? "|" : " ");
    cout.flush();
}

// ============================================================
//  DIFF-BASED DISPLAY — only paints what changed since last call.
//
//  Each call:0

//    1. Erase the OLD mouse position (restore its real content).
//    2. Paint the NEW mouse position with "R".
//    3. Update any wall that was just eaten (caller sets lastWall*).
// ============================================================
void Maze::display(int mouseR, int mouseC)
{
    // --- Erase old mouse position ---
    if (prevMouseR >= 0)
        updateCell(prevMouseR, prevMouseC, -1, -1); // -1,-1 = no mouse

    // --- Paint new mouse position ---
    updateCell(mouseR, mouseC, mouseR, mouseC);

    // --- Update status line with stack depth ---
    int statusLine = 2 + 2 * R + 1;
    moveTo(statusLine + 1, 1);
    cout << "  stack: " << statusText << "        ";
    cout.flush();

    prevMouseR = mouseR;
    prevMouseC = mouseC;
}

// ============================================================
//  HELPERS — neighbours
// ============================================================
vector<pair<int, int>> Maze::unvisitedNeighbours(int r, int c) const
{
    vector<pair<int, int>> nb;
    if (r + 1 < R && !visited[r + 1][c])
        nb.push_back({r + 1, c});
    if (r - 1 >= 0 && !visited[r - 1][c])
        nb.push_back({r - 1, c});
    if (c + 1 < C && !visited[r][c + 1])
        nb.push_back({r, c + 1});
    if (c - 1 >= 0 && !visited[r][c - 1])
        nb.push_back({r, c - 1});
    return nb;
}

vector<pair<int, int>> Maze::openNeighbours(int r, int c,
                                            const vector<vector<bool>> &seen) const
{
    vector<pair<int, int>> nb;
    if (r + 1 < R && !seen[r + 1][c] && !northWall[r][c])
        nb.push_back({r + 1, c});
    if (r - 1 >= 0 && !seen[r - 1][c] && !northWall[r - 1][c])
        nb.push_back({r - 1, c});
    if (c + 1 < C && !seen[r][c + 1] && !eastWall[r][c])
        nb.push_back({r, c + 1});
    if (c - 1 >= 0 && !seen[r][c - 1] && !eastWall[r][c - 1])
        nb.push_back({r, c - 1});
    return nb;
}

// ============================================================
//  ENTRY / EXIT GAPS
// ============================================================
void Maze::openEntryExit()
{
    // Set the gap flags — drawFull() reads these when rendering
    // the outer walls, so no manual terminal writes needed here.
    entryRow = startR;
    exitRow = endR;
}

// ============================================================
//  BONUS — eat one extra wall
// ============================================================
void Maze::eatExtraWall()
{
    if (R < 3 || C < 3)
        return;

    int r = 1 + rand() % (R - 2);
    int c = 1 + rand() % (C - 2);

    int dir = rand() % 4;

    // UP
    if (dir == 0 && r < R - 1)
    {
        northWall[r][c] = false;
        updateNorthWall(r, c);
    }

    // DOWN
    else if (dir == 1 && r > 1)
    {
        northWall[r - 1][c] = false;
        updateNorthWall(r - 1, c);
    }

    // RIGHT
    else if (dir == 2 && c < C - 1)
    {
        eastWall[r][c] = false;
        updateEastWall(r, c);
    }

    // LEFT
    else if (dir == 3 && c > 1)
    {
        eastWall[r][c - 1] = false;
        updateEastWall(r, c - 1);
    }
}
// ============================================================
//  MAZE GENERATION
// ============================================================
void Maze::generateMaze()
{
    cout << HIDE_CURSOR;

    int r = startR, c = startC;
    visited[r][c] = true;
    genStack.push({r, c});

    // Draw the initial full grid (all walls intact) once
    drawFull(r, c);

    int moves = 0;

    while (!genStack.empty())
    {
        r = genStack.top().first;
        c = genStack.top().second;

        // Update status text then redraw only what changed
        ostringstream ss;
        ss << genStack.size();
        statusText = "Gen  depth=" + ss.str();
        display(r, c);

        delay(120); // 120 ms per step — slow enough to watch R move cell by cell

        vector<pair<int, int>> nb = unvisitedNeighbours(r, c);

        if (!nb.empty())
        {
            int idx = rand() % (int)nb.size();
            int nr = nb[idx].first;
            int nc = nb[idx].second;

            // Remove wall and immediately redraw just that wall segment
            if (nr == r + 1)
            {
                northWall[r][c] = false;
                updateNorthWall(r, c);
            }
            else if (nr == r - 1)
            {
                northWall[nr][nc] = false;
                updateNorthWall(nr, nc);
            }
            else if (nc == c + 1)
            {
                eastWall[r][c] = false;
                updateEastWall(r, c);
            }
            else
            {
                eastWall[nr][nc] = false;
                updateEastWall(nr, nc);
            }

            visited[nr][nc] = true;
            genStack.push({nr, nc});

            if (++moves % 20 == 0)
                eatExtraWall();
        }
        else
        {
            genStack.pop();
        }
    }

    openEntryExit();

    // Final mouse position — erase it so the start cell looks clean
    if (prevMouseR >= 0)
        updateCell(prevMouseR, prevMouseC, -1, -1);

    // Status
    int statusLine = 2 + 2 * R + 2;
    moveTo(statusLine, 1);
    cout << "  Maze generated!  Press Enter to solve...     ";
    cout.flush();

    cout << SHOW_CURSOR;
    cin.get();
    cout << HIDE_CURSOR;

    // Reset cell states and prev mouse for the solver phase
    for (int i = 0; i < R; ++i)
        for (int j = 0; j < C; ++j)
            cellState[i][j] = UNVISITED;
    prevMouseR = prevMouseC = -1;

    // Full redraw so the solver starts from a clean picture
    drawFull(startR, startC);
}

// ============================================================
//  MAZE SOLVER
// ============================================================
void Maze::solveMaze()
{
    stack<pair<int, int>> solveStack;
    vector<vector<bool>> seen(R, vector<bool>(C, false));

    int r = startR, c = startC;
    solveStack.push({r, c});
    seen[r][c] = true;
    cellState[r][c] = PATH;
    updateCell(r, c, r, c);

    while (!solveStack.empty())
    {
        r = solveStack.top().first;
        c = solveStack.top().second;

        ostringstream ss;
        ss << solveStack.size();
        statusText = "Solve depth=" + ss.str();
        display(r, c);

        delay(200); // 200 ms per step — you can follow every single move

        if (r == endR && c == endC)
        {
            // Final flash: paint the path bright then show message
            int statusLine = 2 + 2 * R + 2;
            moveTo(statusLine, 1);
            cout << "  *** Maze solved! ***                    ";
            cout.flush();
            cout << SHOW_CURSOR;
            return;
        }

        vector<pair<int, int>> nb = openNeighbours(r, c, seen);

        if (!nb.empty())
        {
            int idx = rand() % (int)nb.size();
            int nr = nb[idx].first;
            int nc = nb[idx].second;

            seen[nr][nc] = true;
            cellState[nr][nc] = PATH;
            // Paint the new path cell immediately (before next display call)
            updateCell(nr, nc, nr, nc);
            solveStack.push({nr, nc});
        }
        else
        {
            // Dead end — turn blue, backtrack
            cellState[r][c] = DEAD;
            updateCell(r, c, -1, -1); // repaint as blue B (no mouse overlay)
            solveStack.pop();
        }
    }

    int statusLine = 2 + 2 * R + 2;
    moveTo(statusLine, 1);
    cout << "  No solution found.                     ";
    cout << SHOW_CURSOR;
    cout.flush();
}