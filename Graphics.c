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

void SpawnBullet(int posX, int posY, Input dir, Header *header) {
  if(header->bulletCount >= NR_MAX_BULLETS) return;

  Bullet newBUllet;
  newBUllet.Coordinates.PosX = posX;
  newBUllet.Coordinates.PosY = posY;
  newBUllet.damage = rand() % BULLET_DMG_MAX;
  newBUllet.facingDir = dir;

  header->bulletStack[header->bulletCount] = newBUllet;
  
  header->bulletCount++;
}

void DestroyBullet(Header *header, Cell *map, int *index) {
  header->bulletStack[*index] = header->bulletStack[header->bulletCount - 1];
  header->bulletCount--;

  (*index)--;
}

void DamagePlayer(Header *header, int j) {
  header->player[j].hp -= BULLET_DMG_BASE + (rand() % BULLET_DMG_MAX);
}

void MoveBullets(Header *header, Cell *map) {
  Bullet localBullet; // so i dont have to write prose every check
  for(int i = 0; i < header->bulletCount; i++) {
    localBullet = header->bulletStack[i];

    if (localBullet.facingDir == none) {
        DestroyBullet(header, map, &i);
        continue;
    }

    int hit = 0;
    for(int j = 0; j < header->CurrentNrPlayers; j++)
      if(localBullet.Coordinates.PosX == header->player[j].Coordinates.PosX && localBullet.Coordinates.PosY == header->player[j].Coordinates.PosY) {//////////DAMAGE PLAYER!!!
        header->bulletStack[i].facingDir = none;        
        DamagePlayer(header, j);

        hit = 1;
      }
      if(hit) continue;


      switch(localBullet.facingDir)
      {
        case up:
          if(localBullet.Coordinates.PosY - 1 < 0) {
            DestroyBullet(header, map, &i);
            break;
          }
          if(map[(localBullet.Coordinates.PosY - 1) * MAP_SIZE_SIDE + localBullet.Coordinates.PosX].value != WALL) {
            header->bulletStack[i].Coordinates.PosY--;
            break;
          }
          header->bulletStack[i].facingDir = none;
        break;

        case down:
          if(localBullet.Coordinates.PosY + 1 >= MAP_SIZE_SIDE) {
            DestroyBullet(header, map, &i);
            break;
          }
          if(map[(localBullet.Coordinates.PosY + 1) * MAP_SIZE_SIDE + localBullet.Coordinates.PosX].value != WALL) {
            header->bulletStack[i].Coordinates.PosY++;
            break;
          }
          header->bulletStack[i].facingDir = none;
        break;

        case left:
          if(localBullet.Coordinates.PosX - 1 < 0) {
            DestroyBullet(header, map, &i);
            break;
          }
          if(map[(localBullet.Coordinates.PosY) * MAP_SIZE_SIDE + (localBullet.Coordinates.PosX - 1)].value != WALL) {
            header->bulletStack[i].Coordinates.PosX--;
            break;
          }
          header->bulletStack[i].facingDir = none;
        break;

        case right:
          if(localBullet.Coordinates.PosX + 1 >= MAP_SIZE_SIDE) {
            DestroyBullet(header, map, &i);
            break;
          }
          if(map[(localBullet.Coordinates.PosY) * MAP_SIZE_SIDE + (localBullet.Coordinates.PosX + 1)].value != WALL) {
            header->bulletStack[i].Coordinates.PosX++;
            break;
          }
          header->bulletStack[i].facingDir = none;
        break;
        
        default:
          break;
      }
    } 
}

void Graphics(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player) {
  int move = 0;

  int ch = wgetch(board_win);
  flushinp();
  if (ch == player->playerControls.up && player->Coordinates.PosY > 0) player->LastMove = up;
  else if (ch == player->playerControls.down && player->Coordinates.PosY < MAP_SIZE_SIDE - 1) player->LastMove = down;
  else if (ch == player->playerControls.left && player->Coordinates.PosX > 0) player->LastMove = left;
  else if (ch == player->playerControls.right && player->Coordinates.PosX < MAP_SIZE_SIDE - 1) player->LastMove = right;
  else if (ch == player->playerControls.fire) player->LastMove = fire;
  else if (ch == 27) /*ESC*/ shutdownGLB = 1;

  currentCycles++;
  if(currentCycles == cyclesToPlayerUpdate) {
    move = 1;
    currentCycles = 0;
  }

  usleep(100000 / GAME_SPEED_MULTIPLIER); // 100ms * gamespeed
  sem_wait(lock);

  if(shutdownGLB) header->shutdown = 1;
  if(header->shutdown) shutdownGLB = 1;

  if(move) {
    
    switch (player->LastMove)
    {
    case up:
        if(map[(player->Coordinates.PosY - 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value == ' ') player->Coordinates.PosY -= 1;
        player->facingDir = up;
      break;

    case down:
        if(map[(player->Coordinates.PosY + 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value == ' ') player->Coordinates.PosY += 1;
        player->facingDir = down;
      break;

    case left:
        if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX - 1)].value == ' ') player->Coordinates.PosX -= 1;
        player->facingDir = left;
      break;

    case right:
        if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX + 1)].value == ' ') player->Coordinates.PosX += 1;
        player->facingDir = right;
      break;
    
    case fire:
        switch (player->facingDir)
        {
          case up:
            if(player->Coordinates.PosY - 1 >= 0)
              if(map[(player->Coordinates.PosY - 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value != WALL) {SpawnBullet(player->Coordinates.PosX, player->Coordinates.PosY-1, up, header);}
            break;
          case down:
            if(player->Coordinates.PosY + 1 < MAP_SIZE_SIDE)
              if(map[(player->Coordinates.PosY + 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value != WALL) {SpawnBullet(player->Coordinates.PosX, player->Coordinates.PosY+1, down, header);}
            break;
          case left:
            if(player->Coordinates.PosX - 1 >= 0)
              if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX - 1)].value != WALL) {SpawnBullet(player->Coordinates.PosX-1, player->Coordinates.PosY, left, header);}
            break;      
          case right:
            if(player->Coordinates.PosX + 1 < MAP_SIZE_SIDE)
              if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX + 1)].value != WALL) {SpawnBullet(player->Coordinates.PosX+1, player->Coordinates.PosY, right, header);}
            break;
            
          default:
            break;
        }
      break;

    default:
      break;
    }

    map[header->player[player_id].Coordinates.PosY * MAP_SIZE_SIDE + header->player[player_id].Coordinates.PosX].value = ' ';
    header->player[player_id].Coordinates.PosX = player->Coordinates.PosX;
    header->player[player_id].Coordinates.PosY = player->Coordinates.PosY;
    map[header->player[player_id].Coordinates.PosY * MAP_SIZE_SIDE + header->player[player_id].Coordinates.PosX].value = player->playerCharacter;

    player->LastMove = none;
  }

  if(player_id == 0)
  MoveBullets(header, map);


  erase();
  werase(board_win);
  box(board_win, 0, 0);
  for(int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) {
    if(map[i].value == WALL) {
      mvwaddch(board_win, map[i].Coordinates.PosY + 1, map[i].Coordinates.PosX + 1, ACTUAL_WALL);
      continue;
    }
    mvwaddch(board_win, map[i].Coordinates.PosY + 1, map[i].Coordinates.PosX + 1, map[i].value);
  }
  for(int i = 0; i < header->bulletCount; i++) {
    char tex = (header->bulletStack[i].facingDir == none) ? '*' : BULLET;
    mvwaddch(board_win, header->bulletStack[i].Coordinates.PosY + 1, header->bulletStack[i].Coordinates.PosX + 1, tex);
  }
  mvprintw(0, 0, " Hp: %d\n Player Character: %c\n Player id: %d", header->player[player_id].hp, header->player[player_id].playerCharacter, player_id);

  wnoutrefresh(stdscr);
  wnoutrefresh(board_win);

  doupdate(); 

  player->hp = header->player[player_id].hp;
  header->player[player_id] = *player;

  sem_post(lock);
}

#endif