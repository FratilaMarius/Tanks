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

#ifndef GRAPHICS_C
#define GRAPHICS_C
#include <ncurses.h>
#include "Defines.h"

#define BORDER_OFFSET 1

int CheckConnected(Vector2 spawn1, Vector2 spawn2, Cell *map) {
  Vector2 toCheck[MAP_SIZE_SIDE * MAP_SIZE_SIDE];
  int checked[MAP_SIZE_SIDE * MAP_SIZE_SIDE] = {0};
  int top = 1;
  toCheck[0] = spawn1;

  Vector2 checking;


  int versorX[4] = {-1, 0, 0, 1};
  int versorY[4] = {0, -1, 1, 0};

  checked[spawn1.PosY * MAP_SIZE_SIDE + spawn1.PosX] = 1;
  while(top) {
    top--;
    checking = toCheck[top];
    if(checking.PosX < 0 || checking.PosY < 0 || checking.PosX >= MAP_SIZE_SIDE || checking.PosY >= MAP_SIZE_SIDE) continue;

    if(checking.PosY == spawn2.PosY && checking.PosX == spawn2.PosX) return 1;

    for(int k = 0; k < 4; k++) {
      if(checking.PosY + versorY[k] < 0 || checking.PosY + versorY[k] >= MAP_SIZE_SIDE 
        || checking.PosX + versorX[k] < 0 || checking.PosX + versorX[k] >= MAP_SIZE_SIDE) continue;

      if(checked[(checking.PosY + versorY[k]) * MAP_SIZE_SIDE + (checking.PosX + versorX[k])] == 1) continue;

      if(map[(checking.PosY + versorY[k]) * MAP_SIZE_SIDE + checking.PosX + versorX[k]].value != WALL) {
        checked[(checking.PosY + versorY[k]) * MAP_SIZE_SIDE + (checking.PosX + versorX[k])] = 1;

        toCheck[top].PosX = checking.PosX + versorX[k];
        toCheck[top].PosY = checking.PosY + versorY[k];
        top++;
      }
    }
  } 

  return 0;
}

static void ClearAround(Vector2 spawn, Cell *map) {
    int versorX[4] = {-1, 0, 0, 1};
    int versorY[4] = {0, -1, 1, 0};

    map[spawn.PosY * MAP_SIZE_SIDE + spawn.PosX].value = ' ';
    for (int k = 0; k < 4; k++) {
        int nx = spawn.PosX + versorX[k], ny = spawn.PosY + versorY[k];
        if (nx < 0 || ny < 0 || nx >= MAP_SIZE_SIDE || ny >= MAP_SIZE_SIDE) continue;
        map[ny * MAP_SIZE_SIDE + nx].value = ' ';
    }
}

void FillMap(Vector2 spawn1, Vector2 spawn2, Cell *map, sem_t *lock) {
    int attempts = -1;
    do {
        attempts++;
        for(int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++)
            map[i].value = RNG_Hit(WALL_CHANCE) ? WALL : ' ';
        ClearAround(spawn1, map);
        ClearAround(spawn2, map);
    } while(!CheckConnected(spawn1, spawn2, map) && attempts < MAX_ATTEMPTS);

    if (!CheckConnected(spawn1, spawn2, map))
        for (int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) map[i].value = ' ';
  }

int shutdownGLB = 0;

void Quit(Header *header, Cell *map, WINDOW *board_win, sem_t *lock, const char *shm_name, int player_id) {
    delwin(board_win);
    endwin();

    sem_wait(lock);
    Vector2 p = header->player[player_id].Coordinates;
    map[p.PosY * MAP_SIZE_SIDE + p.PosX].value = ' ';
    int last; 
    last = (--header->CurrentNrPlayers == 0) ? 1 : 0;
    sem_post(lock);

    munmap(header, SHM_SIZE);
    sem_close(lock);

    if (last) {
        shm_unlink(shm_name);
        sem_unlink(SEMAPHORE);
    }

    exit(0);
}

void Graphics(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player) {

  int ch = wgetch(board_win);
  flushinp();
  if (ch == player->playerControls.up && player->Coordinates.PosY > 0) player->LastMove = up;
  else if (ch == player->playerControls.down && player->Coordinates.PosY < MAP_SIZE_SIDE - 1) player->LastMove = down;
  else if (ch == player->playerControls.left && player->Coordinates.PosX > 0) player->LastMove = left;
  else if (ch == player->playerControls.right && player->Coordinates.PosX < MAP_SIZE_SIDE - 1) player->LastMove = right;
  else if (ch == player->playerControls.fire) player->LastMove = fire;
  else if (ch == 27) /*ESC*/ shutdownGLB = 1;
  else player->LastMove = none;

  sem_wait(lock);
  if(shutdownGLB) header->shutdown = 1;
  if(header->shutdown) shutdownGLB = 1;

  switch (player->LastMove)
  {
  case up:
      if(map[(player->Coordinates.PosY - 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value == ' ') player->Coordinates.PosY -= 1;
    break;

  case down:
      if(map[(player->Coordinates.PosY + 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value == ' ') player->Coordinates.PosY += 1;
    break;

  case left:
      if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX - 1)].value == ' ') player->Coordinates.PosX -= 1;
    break;

  case right:
      if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX + 1)].value == ' ') player->Coordinates.PosX += 1;
    break;


  default:
    break;
  }

  map[header->player[player_id].Coordinates.PosY * MAP_SIZE_SIDE + header->player[player_id].Coordinates.PosX].value = ' ';
  header->player[player_id].Coordinates.PosX = player->Coordinates.PosX;
  header->player[player_id].Coordinates.PosY = player->Coordinates.PosY;
  map[header->player[player_id].Coordinates.PosY * MAP_SIZE_SIDE + header->player[player_id].Coordinates.PosX].value = player->playerCharacter;




  werase(board_win);
  box(board_win, 0, 0);
  for(int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) {
    mvwaddch(board_win, map[i].Coordinates.PosY + 1, map[i].Coordinates.PosX + 1, map[i].value);
  }

  
  wrefresh(board_win);  

  sem_post(lock);
  usleep(50000);
}

#endif