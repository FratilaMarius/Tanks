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

#define NR_MAX_PLAYERS 2
#define MAP_SIZE_SIDE 10 // nr of cells
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
} Direction;

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


#define WALL '0'
typedef struct {
    Vector2 Coordinates;
    char value;
} Cell;

typedef struct {
    Vector2 Coordinates;
    char playerCharacter;
    Controls playerControls;
    Direction LastMove;
} Player;

typedef struct {
    int CurrentNrPlayers;
    Player player[NR_MAX_PLAYERS];
    int shutdown;
} Header;

#endif 