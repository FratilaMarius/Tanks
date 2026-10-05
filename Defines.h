#ifndef DEFINES_H
#define DEFINES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define NR_MAX_PLAYERS 2
#define MAP_SIZE_SIDE 64 // nr of cells

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
} Player;

typedef struct {
    int CurrentNrPlayers;
    Player player[NR_MAX_PLAYERS];
} Header;

#endif 