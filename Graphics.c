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

#ifndef GRAPHICS_C
#define GRAPHICS_C
#include <ncurses.h>
#include "Defines.h"

#define BORDER_OFFSET 1

int shutdown = 0;

void Graphics(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player) {

  int ch = wgetch(board_win);
  flushinp();
  if (ch == player->playerControls.up && player->Coordinates.PosY > 0) player->LastMove = up;
  else if (ch == player->playerControls.down && player->Coordinates.PosY < MAP_SIZE_SIDE - 1) player->LastMove = down;
  else if (ch == player->playerControls.left && player->Coordinates.PosX > 0) player->LastMove = left;
  else if (ch == player->playerControls.right && player->Coordinates.PosX < MAP_SIZE_SIDE - 1) player->LastMove = right;
  else if (ch == player->playerControls.fire) player->LastMove = fire;
  // else if (ch == 'q')
  else player->LastMove = none;

  sem_wait(lock);
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