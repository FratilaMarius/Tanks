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
#endif

///////////////////////////////////////////////////////////////
#ifndef DEFINES_H
#define DEFINES_H

#define NR_MAX_PLAYERS 2
#define MAP_SIZE_SIDE 10 // nr of cells

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
} Header;

#endif 