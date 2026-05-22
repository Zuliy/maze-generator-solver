# Maze Generator & Solver

A C++ console application that **generates** a perfect rectangular maze using a stack-based depth-first search ("mouse") algorithm, then **solves** it using an animated backtracking DFS solver.

---

## How It Works

### Data Structure

Each cell is described by two boolean arrays:

```cpp
northWall[R][C]   // true → top wall of cell (row,col) is solid
eastWall [R][C]   // true → right wall of cell (row,col) is solid
```

**Phantom row convention** (as per the assignment specification):  
Row `0` is treated as a phantom row below the visible maze. Its `northWall[0][j]` values form the **bottom edge** of the maze. Similarly, `eastWall[i][0]` controls gaps in the **left edge**. Rows are displayed top-to-bottom (`R-1` first, `0` last) so the phantom row naturally produces the bottom boundary.

---

### Maze Generation — Stack-based DFS Mouse

1. Place an invisible "mouse" at the starting cell; mark it visited; push it onto a `stack`.
2. While the stack is not empty:
   - Peek at the top cell.
   - Collect all **unvisited** neighbours (up/down/left/right within bounds).
   - If any exist: pick one **at random**, remove the wall between them, mark the neighbour visited, push it. Other unvisited neighbours are implicitly saved because the current cell stays on the stack.
   - If none exist (dead end): **pop** — the mouse backtracks.
3. When the stack empties, every cell has been visited exactly once and is connected to every other cell by a unique path → **perfect maze**.
4. Entry and exit gaps are opened on the left and right outer walls at the start and end rows.

The `display()` call inside the generation loop lets you watch the maze being carved in real time.

**Bonus — cycle introduction:** Every 20 wall-eating moves, one extra wall is randomly removed. This introduces a cycle that defeats the classic "shoulder-to-the-wall" traversal rule, making the maze more challenging.

---

### Maze Solving — Backtracking DFS

1. Push the start cell, mark it `PATH` (red dot `·`).
2. While the stack is not empty:
   - Peek at the top cell (draw it in **red** `R`).
   - If it is the goal → **solved!**
   - Collect open (wall-free) neighbours not yet visited.
   - If any exist: pick one at random, mark it `PATH`, push.
   - If none (dead end): mark current cell `DEAD` (**blue** `B`), pop.

---

### Question — Stack vs Queue for Generation

Using a **stack** produces the classic recursive-backtracker maze: long winding corridors with relatively few branches. The mouse goes deep before backtracking, so passages tend to be long and winding.

Using a **queue** (BFS) would create mazes with many short branches radiating outward from the start, more like a river delta. Paths would be shorter but the maze would feel "bushier" and less labyrinthine. A queue also guarantees that nearby cells are carved before distant ones, which changes the visual texture of generation dramatically — you'd see a spreading frontier rather than a single tentacle burrowing through the grid.

**For this assignment, a stack is the better choice** because it produces more interesting, longer paths that look and feel like traditional mazes.

---

## Building & Running

```bash
# Compile (g++ / clang++)
g++ -std=c++17 -O2 -o maze src/main.cpp src/Maze.cpp

# Run
./maze
```

Requirements: any C++17-compatible compiler. No external libraries.

---

## Display Legend

| Symbol | Meaning |
|--------|---------|
| `R` (red) | Current mouse position |
| `G` (green) | Goal / end cell |
| `.` (red) | Active solution path |
| `B` (blue) | Dead-end cell (backtracked) |
| `+---+` | Intact walls |
| `+   +` | Removed wall (passage) |

---

## File Structure

```
maze/
├── include/
│   └── Maze.h          ← class declaration + data structure docs
├── src/
│   ├── Maze.cpp        ← generator, solver, display
│   └── main.cpp        ← entry point
└── README.md
```

---

## Bonus Feature

Setting `extraWallCounter % 20 == 0` in `generateMaze()` causes the mouse to randomly eat one extra wall approximately once every 20 steps. This creates **cycles** in what would otherwise be a tree-structured maze. As a result, the "shoulder-to-the-wall" rule (always keep your hand on the left wall) no longer guarantees reaching the exit — the algorithm can loop around a cycle indefinitely without ever finding the goal.