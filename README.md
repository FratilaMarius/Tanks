# Tanks!

A two-player tank duel for the terminal, written in C.

Each player runs **their own process** of the same program. The processes share the game board through **POSIX shared memory** and coordinate with a **named semaphore**. The interface is drawn with **ncurses**.


## Features

- Two players, one process each, one shared board
- Random walls, with a guaranteed path between the two spawn points
- Animated bullets that stop at walls, tanks and the edge of the board
- HP and damage, a win/lose screen, and a restart that both players must agree to
- Round-win counter
- Clean shutdown that removes the shared memory and the semaphore


## Requirements (for playing)
- A TMUX terminal of at least 27 x 27 characters (80 x 30 or larger is recommended), syncronize-panes setting on



## Requirements (for recompiling)

- Linux (developed on Ubuntu). It uses POSIX shared memory and named semaphores, which live under `/dev/shm`.
- GCC and the ncurses development headers
```bash
sudo apt install build-essential libncurses-dev
```
- Under ShellScripts you will find a file that rebuilds the game for you.

## Run
Inside a terminal: './Tanks <board name> <your player character> <your 4 movement keys> <your shooting key>'
Start one instance per player. **Both must use the same board name.**

```bash
# terminal 1 (creates the game, becomes player 0)
./tanks arena A wasd f

# terminal 2 (joins the game, becomes player 1)
./tanks arena B ijkl h
```

The first process to start creates the game and generates the map. The second one joins it. A third one is refused with `Max players reached`.

### Playing on one keyboard

A terminal only receives keystrokes while it has focus, so two plain windows can't both listen to one keyboard. The easiest way is `tmux`:

1. Start `tmux` and split the window into two panes (`Ctrl-b` then `%`).
2. Run one instance per pane.
3. Turn on broadcasting: `Ctrl-b`, then `:setw synchronize-panes on`.

## Controls

| Key | Action |
|-----|--------|
| your movement keys | move one cell (up / left / down / right); also sets the direction you face |
| your fire key | shoot one bullet in the direction you face |
| `SPACE` (end screen) | vote to restart; **both** players must press it |
| `ESC` | quit; this ends the game for both players |


## Technical stuff

```
 terminal 1                            terminal 2
 +-----------------+                   +-----------------+
 | ./tanks arena   |                   | ./tanks arena   |
 | player 0        |                   | player 1        |
 | (creator)       |                   | (joiner)        |
 +--------+--------+                   +--------+--------+
          |   sem_wait / sem_post (named semaphore)  |
          v                                          v
 +-------------------------------------------------------------+
 |  POSIX shared memory   /dev/shm/<board>                     |
 |    Header { players[2], bullets[25], flags ... }            |
 |    Cell map[20 * 20]                                        |
 +-------------------------------------------------------------+
```

**Starting up.** Every process opens the semaphore, then tries `shm_open(board, O_CREAT | O_EXCL)`. Exactly one process succeeds. That one becomes player 0: it sizes the segment with `ftruncate` and generates the map. Any other process attaches to the existing segment as player 1. Both map the segment once with `mmap(MAP_SHARED)`. The header sits at the start of the mapping and the map cells follow it.

**The frame loop.** Each process repeats this about every 50 ms:

1. Read one key without blocking (`nodelay`) and remember the latest command.
2. Sleep for the rest of the frame.
3. Take the semaphore. On every third frame, apply the remembered move or shot. Player 0 also advances all bullets and applies damage. Then copy the shared state into a local snapshot.
4. Release the semaphore and draw from the snapshot. Drawing never holds the lock.

**Locking.** One named semaphore (`/semaphore`, initial value 1) acts as a mutex. Every read-modify-write of the shared board, players or bullets happens between `sem_wait` and `sem_post`.

**Bullets.** They live in a fixed-size array in shared memory plus a counter. Removing a bullet copies the last one into its slot (O(1), no gaps). A bullet that hit something turns into an explosion for one frame and is then removed.

**Map generation.** Each cell becomes a wall with a configurable probability, the cells around both spawns are cleared, and a flood fill (an explicit stack where every cell is pushed at most once) checks that the spawns are connected. If they aren't, the map is regenerated, up to a limit. After that limit it falls back to an empty board.

**Shutdown.** `ESC` sets a shared flag, so both processes quit. Each process removes its tank, decrements the player count and unmaps the memory. The **last** one out calls `shm_unlink` and `sem_unlink`.

## Project structure

| Module | Responsibility |
|--------|----------------|
| `main.c` | Argument parsing, shared memory and semaphore setup, main loop |
| `Defines.h` | Constants, shared structs (`Header`, `Player`, `Bullet`, `Cell`), tunables |
| `Graphics.c/.h` | ncurses window setup, rendering, the end screen, per-frame input and movement |
| `GameLogic.c/.h` | `SetupGame`, bullets (spawn, move, destroy), damage |
| `MathLogic.c/.h` | Random numbers, map generation, flood-fill connectivity check |
| `Utility.c/.h` | Argument parsing, shutdown and cleanup, signal handler |

## Configuration

These can be changed in `Defines.h`:

| Constant | Default | Meaning |
|----------|---------|---------|
| `MAP_SIZE_SIDE` | 20 | board width and height in cells |
| `WALL_CHANCE` | 15 | percentage of cells that become walls |
| `MAX_ATTEMPTS` | 50 | map regeneration attempts before giving up |
| `STARTING_HP` | 100 | HP at the start of each round |
| `BULLET_DMG_BASE`, `BULLET_DMG_MAX` | 5, 15 | damage is `BASE + rand() % MAX` |
| `NR_MAX_BULLETS` | 25 | bullets that can be in flight at once |
| `GAME_SPEED_MULTIPLIER` | 2 | frame time is 100 ms divided by this |
| `cyclesToPlayerUpdate` | 3 | frames between tank moves |
| `BULLET`, `VERT_BULLET`, `HORZ_BULLET`, `EXPLOSION` | `.`, `\|`, `_`, `*` | characters used for bullets |

## Troubleshooting

- **`Max players reached` right after a crash, or the game hangs at startup.** A crashed run leaves its shared memory and semaphore behind. Remove them by running the cleanup.sh script.'
- **Keys do nothing in the second window.** Only the focused terminal receives keys. See "Playing on one keyboard".
- **Crash after rebuilding with a changed struct.** An old segment of a different size is still in `/dev/shm`. Remove it as above.

## Limitations

- Exactly two players, though this would be trivial to change (only some consts in Defines).
- Linux only, because of `/dev/shm` and POSIX IPC.
- The semaphore name is fixed, so run one game at a time.

Author: *Fratila Marius*
