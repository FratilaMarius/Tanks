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

#include "Defines.h"
#include "GameLogic.h"

#ifndef NCURSES
#define NCURSES
#include "ncurses.h"
#endif

#ifndef GRAPHICS_H
#define GRAPHICS_H


WINDOW *SetupWindow();


void DrawBigText(const char *text, int color);


void DrawEndScreen(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player, Vector2 spawn1, Vector2 spawn2);


void Graphics(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player, Vector2 spawn1, Vector2 spawn2);


#endif