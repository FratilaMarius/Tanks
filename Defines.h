#ifndef LIBS
#define LIBS
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#endif

///////////////////////////////////////////////////////////////
#ifndef DEFINES_H
#define DEFINES_H

// map / graphics
#define MAP_SIZE_SIDE 20 // nr of cells
#define WALL_CHANCE 15
#define MAX_ATTEMPTS 50

#define WALL '#' //// DO NOT CHANGE!!!!!!
#define ACTUAL_WALL ACS_BLOCK

#define BULLET '.'
#define EXPLOSION '*'
#define VERT_BULLET '|'
#define HORZ_BULLET '_'


// wining
extern int winner;
extern int wins;
extern int inEndScreen;
extern int rounds;


// game mechanics consts
#define NR_MAX_PLAYERS 2
#define STARTING_HP 100

#define SPAWN1X (MAP_SIZE_SIDE / 4)
#define SPAWN1Y (MAP_SIZE_SIDE / 4)
#define SPAWN2X (3 * MAP_SIZE_SIDE / 4)
#define SPAWN2Y (3 * MAP_SIZE_SIDE / 4)

#define NR_MAX_BULLETS 25
#define BULLET_DMG_MAX 15
#define BULLET_DMG_BASE 5


// management
#define SHM_SIZE (sizeof(Header) + MAP_SIZE_SIDE * MAP_SIZE_SIDE * sizeof(Cell))
#define SEMAPHORE "/semaphore"

extern int shutdownGLB;

#define GAME_SPEED_MULTIPLIER 2
extern int cyclesToPlayerUpdate;
extern int currentCycles;

extern int localHasVoted; 



typedef struct {
    int PosX;
    int PosY;
} Vector2;

typedef enum {
  up,
  down,
  left, 
  right,

  fire,
  none
} Input;

typedef struct {
    char up;
    char down;
    char left;
    char right;
    char fire;
} Controls;

typedef struct {
    Vector2 Coordinates;
    char value;
} Cell;
extern Cell localMap[MAP_SIZE_SIDE * MAP_SIZE_SIDE];

typedef struct {
    Vector2 Coordinates;
    char playerCharacter;
    Controls playerControls;
    Input LastMove;
    Input facingDir;

    int hp;
} Player;

typedef struct {
  Vector2 Coordinates;
  Input facingDir;
  int damage;
} Bullet;


typedef struct {
  int CurrentNrPlayers;
  Player player[NR_MAX_PLAYERS];

  int shutdown;
  int gameOver;
  int restartVotes;

  int bulletCount;
  Bullet bulletStack[NR_MAX_BULLETS];
} Header;
extern Header localHeader;

extern char board[64];
#endif 