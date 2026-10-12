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

#ifndef MATHLOGIC_H
#define MATHLOGIC_H


int RNG_Hit(const int chance);


static void ClearAround(Vector2 spawn, Cell *map);


int CheckConnected(Vector2 spawn1, Vector2 spawn2, Cell *map);


void FillMap(Vector2 spawn1, Vector2 spawn2, Cell *map, sem_t *lock);


#endif