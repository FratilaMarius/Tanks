#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "Defines.h"

void GetArgs(int argc, char *argv[]) {
  
}

#define SHARED_MEM "/shared_mem"
#define SEMAPHORE "/semaphore"

int main(int argc, char *argv[]) {
  int is_creator = 0;
  
  //==================================================================================================================
  // semaphore:
  sem_t *lock = sem_open(SEMAPHORE, O_CREAT, 0666, 1);
  if (lock == SEM_FAILED) {
    perror("sem_open");
    sem_post(lock);
    exit(EXIT_FAILURE);
  }
  sem_wait(lock);
  //==================================================================================================================

  int sharedMem_fd = shm_open(SHARED_MEM, O_RDWR | O_CREAT | O_EXCL, 0666);
  if (sharedMem_fd == -1 && errno != EEXIST) { // if the maping wasnt created 
    perror("shm_open");
    sem_post(lock);
    exit(EXIT_FAILURE);
  }

  if (sharedMem_fd == -1 && errno == EEXIST) { // if it wasnt created beacause it was there already
    sharedMem_fd = shm_open(SHARED_MEM, O_RDWR, 0666);
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

    is_creator = 1;
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

  if(is_creator) {
    header->CurrentNrPlayers = 1;

    for (int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) {
      map[i].Coordinates.PosX = i % MAP_SIZE_SIDE;
      map[i].Coordinates.PosY = i / MAP_SIZE_SIDE;
      map[i].value = ' ';
    }
  }
  else {
    if(header->CurrentNrPlayers >= NR_MAX_PLAYERS) {
      printf("Max players reached\n");
      sem_post(lock);
      exit(EXIT_FAILURE);
    }
    else header->CurrentNrPlayers++;
  }


  sem_post(lock);
  //==================================================================================================================

  /// cleanup
  /// delete memMap and semaphore files
}

