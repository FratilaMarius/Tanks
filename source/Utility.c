#include "../Utility.h"


void handle_sigint(int sig) {
  shutdownGLB = 1; 
}


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

  strncpy(board, argv[1], sizeof(board) - 1);
}

