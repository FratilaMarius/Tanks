#include "../MathLogic.h"


int RNG_Hit(const int chance) {
  return ((rand() % 100) < chance) ? 1 : 0;
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