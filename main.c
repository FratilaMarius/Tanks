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

#include "Graphics.h"
#include "GameLogic.h"
#include "MathLogic.h"
#include "Utility.h"
#include "Defines.h"


int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sigint);

  srand(time(NULL));
  Player player;

  Vector2 spawn1, spawn2;
  spawn1.PosX = SPAWN1X;  spawn1.PosY = SPAWN1Y;
  spawn2.PosX = SPAWN2X;  spawn2.PosY = SPAWN2Y;


  int player_id = 1;

  GetArgs(argc, argv, &player);

  //==================================================================================================================
  // semaphore:
  sem_t *lock = sem_open(SEMAPHORE, O_CREAT, 0666, 1);
  if (lock == SEM_FAILED) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_wait(lock);
  //==================================================================================================================

  int sharedMem_fd = shm_open(board, O_RDWR | O_CREAT | O_EXCL, 0666);
  if (sharedMem_fd == -1 && errno != EEXIST) { // if the maping wasnt created 
    perror("shm_open");
    sem_post(lock);
    exit(EXIT_FAILURE);
  }

  if (sharedMem_fd == -1 && errno == EEXIST) { // if it wasnt created beacause it was there already
    sharedMem_fd = shm_open(board, O_RDWR, 0666);
    if (sharedMem_fd == -1) {
      perror("shm_open");
      sem_post(lock);
      exit(EXIT_FAILURE);
    }
  }
  else { // creator branch
    if (ftruncate(sharedMem_fd, sizeof(Header) + MAP_SIZE_SIDE * MAP_SIZE_SIDE * sizeof(Cell))) { 
      perror("ftruncate");
      sem_post(lock);
      exit(EXIT_FAILURE);
    }
    player_id = 0;
  }  // bullshittery done to avoid truncating twice cause who knows (and to find the first process)

  //==================================================================================================================
  // splicing:
  void *base = mmap(NULL, sizeof(Header) + MAP_SIZE_SIDE * MAP_SIZE_SIDE * sizeof(Cell), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMem_fd, 0);
  if (base == MAP_FAILED) {
    perror("mmap at base alloc");
    sem_post(lock);
    exit(EXIT_FAILURE);
  }
  Header *header = (Header *)base;
  Cell *map = (Cell *)(base + sizeof(Header));
  close(sharedMem_fd);

  if(player_id == 0) {
    header->CurrentNrPlayers = 1;
    header->shutdown = 0;
  }
  else {
    if(header->CurrentNrPlayers >= NR_MAX_PLAYERS) {
      printf("Max players reached\n");
      munmap(header, SHM_SIZE);
      sem_post(lock); 
      exit(EXIT_FAILURE);
    }
    else header->CurrentNrPlayers++;
  }

  SetupGame(header, map, lock, &player, spawn1, spawn2, player_id);

  sem_post(lock);
  //==================================================================================================================
  // if(is_creator) {
  //   pid_t pid = fork();
  //   if(pid == -1) {
  //     perror("fork");
  //     sem_post(lock);
  //     exit(EXIT_FAILURE);
  //   }
  //   if(pid == 0) {
  //     is_creator = 0;

  WINDOW *board_win = SetupWindow();

  while(1) {
    Graphics(header, map, lock, board_win, player_id, &player, spawn1, spawn2);
    if(shutdownGLB) Quit(header, map, board_win, lock, board, player_id);
  }
  //   }
  // } /// init graphics

  /// cleanup
  /// delete memMap and semaphore files
}

