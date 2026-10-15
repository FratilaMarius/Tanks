#include "../GameLogic.h"


void SetupGame(Header *header, Cell *map, sem_t *lock, Player *player, Vector2 spawn1, Vector2 spawn2, int player_id) { // to be called with locked semaphore  

  if(player_id == 0) {
    header->gameOver--;
    rounds++;

    for (int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) {
      map[i].Coordinates.PosX = i % MAP_SIZE_SIDE;
      map[i].Coordinates.PosY = i / MAP_SIZE_SIDE;
      map[i].value = ' ';
    }

    FillMap(spawn1, spawn2, map, lock);

    player->hp = STARTING_HP;
    player->Coordinates = spawn1;
    player->facingDir = down;
    header->player[0] = *player;
    map[player->Coordinates.PosY * MAP_SIZE_SIDE + player->Coordinates.PosX].value = player->playerCharacter;

    header->bulletCount = 0;
    player->LastMove = none;
  }
  else {
    header->gameOver--;

    player->hp = STARTING_HP;
    player->Coordinates = spawn2;
    header->player[1] = *player;
    player->facingDir = up;
    map[player->Coordinates.PosY * MAP_SIZE_SIDE + player->Coordinates.PosX].value = player->playerCharacter;
    player->LastMove = none;
  }  
  if(header->gameOver <= 0) header->restartVotes = 0;

  // shi gon break if there s more than 2 players  
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

