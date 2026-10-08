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
  if (ch == player->playerControls.up && player->Coordinates.PosY > 0) player->Coordinates.PosY -= 1;
  else if (ch == player->playerControls.down && player->Coordinates.PosY < MAP_SIZE_SIDE) player->Coordinates.PosY += 1;
  else if (ch == player->playerControls.left && player->Coordinates.PosX > 0) player->Coordinates.PosX -= 1;
  else if (ch == player->playerControls.right && player->Coordinates.PosX < MAP_SIZE_SIDE) player->Coordinates.PosX += 1;
  // else if (ch == player->playerControls.fire)  
  // else if (ch == 'q')

  sem_wait(lock);
  header->player[player_id].Coordinates.PosX = player->Coordinates.PosX;
  header->player[player_id].Coordinates.PosY = player->Coordinates.PosY;

  werase(board_win);
  box(board_win, 0, 0);
  for(int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) {
    mvwaddch(board_win, map[i].Coordinates.PosY + 1, map[i].Coordinates.PosX + 1, map[i].value);
  }

  mvwaddch(board_win, header->player[player_id].Coordinates.PosY + BORDER_OFFSET, header->player[player_id].Coordinates.PosX + BORDER_OFFSET, 
           header->player[player_id].playerCharacter);
  
  wrefresh(board_win);  

  sem_post(lock);
  usleep(50000);
}

#endif