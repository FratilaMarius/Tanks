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
#include "MathLogic.h"

#ifndef GAMELOGIC_H
#define GAMELOGIC_H


void SetupGame(Header *header, Cell *map, sem_t *lock, Player *player, Vector2 spawn1, Vector2 spawn2, int player_id);// to be called with locked semaphore  


void SpawnBullet(int posX, int posY, Input dir, Header *header);


void DestroyBullet(Header *header, Cell *map, int *index);


void DamagePlayer(Header *header, int j);


void MoveBullets(Header *header, Cell *map);


#endif