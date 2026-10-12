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

#ifndef NCURSES
#define NCURSES
#include "ncurses.h"
#endif

#ifndef UTILITY_H
#define UTILITY_H


void handle_sigint(int sig);

void Quit(Header *header, Cell *map, WINDOW *board_win, sem_t *lock, const char *shm_name, int player_id);

void GetArgs(int argc, char *argv[], Player *p);


#endif