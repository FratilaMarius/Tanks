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
#endif

///////////////////////////////////////////////////////////////
#ifndef DEFINES_H
#define DEFINES_H

#define WALL '#' //// DO NOT CHANGE!!!!!!
#define BULLET '.'
#define EXPLOSION '*'
#define ACTUAL_WALL ACS_BLOCK

#define GAME_SPEED_MULTIPLIER 2
int cyclesToPlayerUpdate = 3;
int currentCycles = 0;

#define STARTING_HP 100
#define NR_MAX_PLAYERS 2
#define NR_MAX_BULLETS 25
#define BULLET_DMG_MAX 50
#define BULLET_DMG_BASE 10

#define MAP_SIZE_SIDE 20 // nr of cells
#define WALL_CHANCE 15
#define MAX_ATTEMPTS 50

#define SPAWN1X (MAP_SIZE_SIDE / 4)
#define SPAWN1Y (MAP_SIZE_SIDE / 4)

#define SPAWN2X (3 * MAP_SIZE_SIDE / 4)
#define SPAWN2Y (3 * MAP_SIZE_SIDE / 4)

#define SHM_SIZE (sizeof(Header) + MAP_SIZE_SIDE * MAP_SIZE_SIDE * sizeof(Cell))
#define SEMAPHORE "/semaphore"

int RNG_Hit(const int chance) {
  return ((rand() % 100) < chance) ? 1 : 0;
}

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
    int PosX;
    int PosY;
} Vector2;


typedef struct {
    Vector2 Coordinates;
    char value;
} Cell;

typedef struct {
    Vector2 Coordinates;
    char playerCharacter;
    Controls playerControls;
    Input LastMove;
    Input facingDir;

    int hp;
} Player;

typedef struct
{
  Vector2 Coordinates;
  Input facingDir;
  int damage;
} Bullet;


typedef struct {
    int CurrentNrPlayers;
    Player player[NR_MAX_PLAYERS];
    int shutdown;

    int bulletCount;
    Bullet bulletStack[NR_MAX_BULLETS];
} Header;

#endif 