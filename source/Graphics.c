#include "../Graphics.h"


WINDOW *SetupWindow() {
  initscr();
  set_escdelay(25);

  WINDOW *board_win = newwin(MAP_SIZE_SIDE + 2, MAP_SIZE_SIDE + 2, 5, 5);
  cbreak(); 
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);

  keypad(board_win, TRUE);
  nodelay(board_win, TRUE);
  start_color();
  init_pair(3, COLOR_GREEN, COLOR_GREEN);
  init_pair(4, COLOR_RED, COLOR_BLUE);
  init_pair(5, COLOR_BLUE, COLOR_RED);
  init_pair(6, COLOR_YELLOW, COLOR_BLACK);
  init_pair(7, COLOR_RED, COLOR_BLACK); // loser
  init_pair(8, COLOR_GREEN, COLOR_BLACK); // winner

  attron(A_BOLD);             
  wattron(board_win, A_BOLD); 

  return board_win;
}


void DrawBigText(const char *text, int color) {

  int startY = (LINES) / 2;
  int startX = (COLS) / 2 - strlen(text) / 2;

  attron(COLOR_PAIR(color));

  mvprintw(startY, startX, "%s", text);

  attroff(COLOR_PAIR(color));
}


void DrawEndScreen(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player, Vector2 spawn1, Vector2 spawn2) {
  if(winner != 0 && aux_incr_wins == 0) {
    wins++;
    aux_incr_wins = 1;
  }
  erase();  
  werase(board_win);  

  if (winner == 1) {
    DrawBigText("You Win! (Press Space to Restart)", 8);
  } else {
    DrawBigText("You Lose! (Press Space to Restart)", 7);
  }
  
  wnoutrefresh(stdscr);
  // wnoutrefresh(board_win);
  doupdate(); 
  
  int ch = getch();
    if(ch == ' ' && localHasVoted == 0) {
    sem_wait(lock);
    header->restartVotes++;
    sem_post(lock);
    localHasVoted = 1; 
  }
  
  sem_wait(lock);
  if(localHasVoted == 1 && header->restartVotes >= header->CurrentNrPlayers) {
    localHasVoted = 0;
    
    SetupGame(header, map, lock, player, spawn1, spawn2, player_id);
  }
  sem_post(lock);
}


void Graphics(Header *header, Cell *map, sem_t *lock, WINDOW *board_win, int player_id, Player *player, Vector2 spawn1, Vector2 spawn2) {
  int move = 0;

  if(player->hp <= 0) {
    winner = 0;
    header->gameOver = 2;
  }

  if(header->gameOver > 0) {
    DrawEndScreen(header, map, lock, board_win, player_id, player, spawn1, spawn2);
    usleep(100000 / GAME_SPEED_MULTIPLIER);
    return;  
  }
  winner = 1;


  int ch = wgetch(board_win);
  flushinp();
  if (ch == player->playerControls.up && player->Coordinates.PosY > 0) player->LastMove = up;
  else if (ch == player->playerControls.down && player->Coordinates.PosY < MAP_SIZE_SIDE - 1) player->LastMove = down;
  else if (ch == player->playerControls.left && player->Coordinates.PosX > 0) player->LastMove = left;
  else if (ch == player->playerControls.right && player->Coordinates.PosX < MAP_SIZE_SIDE - 1) player->LastMove = right;
  else if (ch == player->playerControls.fire) player->LastMove = fire;
  else if (ch == 27) /*ESC*/ shutdownGLB = 1;

  currentCycles++;
  if(currentCycles == cyclesToPlayerUpdate) {
    move = 1;
    currentCycles = 0;
  }

  usleep(100000 / GAME_SPEED_MULTIPLIER); // 100ms * gamespeed
  sem_wait(lock);

  if(shutdownGLB) header->shutdown = 1;
  if(header->shutdown) shutdownGLB = 1;

  if(move) {
    
    switch (player->LastMove)
    {
    case up:
        if(map[(player->Coordinates.PosY - 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value == ' ') player->Coordinates.PosY -= 1;
        player->facingDir = up;
      break;

    case down:
        if(map[(player->Coordinates.PosY + 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value == ' ') player->Coordinates.PosY += 1;
        player->facingDir = down;
      break;

    case left:
        if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX - 1)].value == ' ') player->Coordinates.PosX -= 1;
        player->facingDir = left;
      break;

    case right:
        if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX + 1)].value == ' ') player->Coordinates.PosX += 1;
        player->facingDir = right;
      break;
    
    case fire:
        switch (player->facingDir)
        {
          case up:
            if(player->Coordinates.PosY - 1 >= 0)
              if(map[(player->Coordinates.PosY - 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value != WALL) {SpawnBullet(player->Coordinates.PosX, player->Coordinates.PosY-1, up, header);}
            break;
          case down:
            if(player->Coordinates.PosY + 1 < MAP_SIZE_SIDE)
              if(map[(player->Coordinates.PosY + 1) * MAP_SIZE_SIDE + player->Coordinates.PosX].value != WALL) {SpawnBullet(player->Coordinates.PosX, player->Coordinates.PosY+1, down, header);}
            break;
          case left:
            if(player->Coordinates.PosX - 1 >= 0)
              if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX - 1)].value != WALL) {SpawnBullet(player->Coordinates.PosX-1, player->Coordinates.PosY, left, header);}
            break;      
          case right:
            if(player->Coordinates.PosX + 1 < MAP_SIZE_SIDE)
              if(map[(player->Coordinates.PosY) * MAP_SIZE_SIDE + (player->Coordinates.PosX + 1)].value != WALL) {SpawnBullet(player->Coordinates.PosX+1, player->Coordinates.PosY, right, header);}
            break;
            
          default:
            break;
        }
      break;

    default:
      break;
    }

    map[header->player[player_id].Coordinates.PosY * MAP_SIZE_SIDE + header->player[player_id].Coordinates.PosX].value = ' ';
    header->player[player_id].Coordinates.PosX = player->Coordinates.PosX;
    header->player[player_id].Coordinates.PosY = player->Coordinates.PosY;
    map[header->player[player_id].Coordinates.PosY * MAP_SIZE_SIDE + header->player[player_id].Coordinates.PosX].value = player->playerCharacter;

    player->LastMove = none;
  }

  if(player_id == 0)
  MoveBullets(header, map);

  //////////////////////////////////////////////////////////////////////////////////////////// Redndering:
  //
  localHeader = *header;
  player->hp = localHeader.player[player_id].hp;
  localHeader.player[player_id] = *player;
  for(int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) localMap[i] = map[i]; 

  sem_post(lock);

  erase();
  werase(board_win);
  box(board_win, 0, 0);
  for(int i = 0; i < MAP_SIZE_SIDE * MAP_SIZE_SIDE; i++) {
    if(localMap[i].value == WALL) {
      wattron(board_win, COLOR_PAIR(3));
      mvwaddch(board_win, localMap[i].Coordinates.PosY + 1, localMap[i].Coordinates.PosX + 1, ACTUAL_WALL);
      wattroff(board_win, COLOR_PAIR(3));
      continue;
    }
    if(localMap[i].value == localHeader.player[0].playerCharacter) {
      wattron(board_win, COLOR_PAIR(4));
      mvwaddch(board_win, localMap[i].Coordinates.PosY + 1, localMap[i].Coordinates.PosX + 1, localMap[i].value);
      wattroff(board_win, COLOR_PAIR(4));
      continue;
    }
    if(localMap[i].value == localHeader.player[1].playerCharacter) {
      wattron(board_win, COLOR_PAIR(5));
      mvwaddch(board_win, localMap[i].Coordinates.PosY + 1, localMap[i].Coordinates.PosX + 1, localMap[i].value);
      wattroff(board_win, COLOR_PAIR(5));
      continue;
    }
      
    mvwaddch(board_win, localMap[i].Coordinates.PosY + 1, localMap[i].Coordinates.PosX + 1, localMap[i].value);
  }
  for(int i = 0; i < localHeader.bulletCount; i++) {
    char tex;
    switch(localHeader.bulletStack[i].facingDir) {
      case up:
      case down:
        tex = VERT_BULLET;
      break;

      case left:
      case right:
        tex = HORZ_BULLET;
      break;

      case none:
        tex = EXPLOSION;
      break;

      default:
        tex = BULLET;
      break;
    }
    wattron(board_win, COLOR_PAIR(6));
    mvwaddch(board_win, localHeader.bulletStack[i].Coordinates.PosY + 1, localHeader.bulletStack[i].Coordinates.PosX + 1, tex);
    wattroff(board_win, COLOR_PAIR(6));
  }
  mvprintw(0, 0, " Hp: %d\n Player Character: %c\n Player id: %d\n Round Wins: %d", localHeader.player[player_id].hp, localHeader.player[player_id].playerCharacter, player_id, wins);

  wnoutrefresh(stdscr);
  wnoutrefresh(board_win);

  doupdate(); 
}