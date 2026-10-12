#include "../Defines.h"


// wining
int winner = 1;
int wins = 0;
int aux_incr_wins = 0;
int rounds = 0;


// management
int shutdownGLB = 0;

int cyclesToPlayerUpdate = 3;
int currentCycles = 0;

int localHasVoted = 0;


Cell localMap[MAP_SIZE_SIDE * MAP_SIZE_SIDE];
Header localHeader;
char board[64];
