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

#include "Graphics.c"
#include "Defines.h"

char board[64];

void GetArgs(int argc, char *argv[], Player *p) {  // Board, display_character, movement + Fire keys
  if(argc != 5 || strlen(argv[3]) != 4 || strlen(argv[4]) != 1 || strlen(argv[2]) != 1) {          ////// TODO: add a check for the board
    printf("Usage: %s <board: MAX 5 CHARS!> <display_character> <movement_commands, eg:WASD> <fire>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  p->playerCharacter = argv[2][0];

  p->playerControls.up = argv[3][0];
  p->playerControls.left = argv[3][1];
  p->playerControls.down = argv[3][2];
  p->playerControls.right = argv[3][3];

  p->playerControls.fire = argv[4][0];

  strncpy(board, argv[1], strlen(argv[1]));
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  Player player;

  Vector2 spawn1, spawn2;
  spawn1.PosX = SPAWN1X;  spawn1.PosY = SPAWN1Y;
  spawn2.PosX = SPAWN2X;  spawn2.PosY = SPAWN2Y;


  int is_creator = 0;
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
    is_creator = 1;
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

  if(is_creator) {
    header->CurrentNrPlayers = 1;
    header->shutdown = 0;

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

  if(player_id == 0) {
    player.Coordinates = spawn1;
    header->player[0] = player;
    map[player.Coordinates.PosY * MAP_SIZE_SIDE + player.Coordinates.PosX].value = player.playerCharacter;

    FillMap(spawn1, spawn2, map, lock);
  }
  else {
    player.Coordinates = spawn2;
    header->player[1] = player;
    map[player.Coordinates.PosY * MAP_SIZE_SIDE + player.Coordinates.PosX].value = player.playerCharacter;
  }                               /// asta crapa dacabag mai multi playeri, imi trebuie alt check ca sa vad ce player e
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
  initscr();
  WINDOW *board_win = newwin(MAP_SIZE_SIDE + 3, MAP_SIZE_SIDE + 3, 0, 0);
  cbreak(); 
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);

  keypad(board_win, TRUE);
  nodelay(board_win, TRUE); 

  while(1) {
    Graphics(header, map, lock, board_win, player_id, &player);
    if(shutdownGLB) Quit(header, map, board_win, lock, board, player_id);
  }
  //   }
  // } /// init graphics

  /// cleanup
  /// delete memMap and semaphore files
}

