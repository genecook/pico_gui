#include <stdio.h>
#include <iostream>
#include <string>
#include "pico/stdlib.h"

#include <chess.h>
#include <chess_engine_gui.h>

namespace PicoStreamPlayer {
  int Play(PicoChess::ChessEngine *the_engine);
}

int main() {
  GuiStartup();
  NewGame();
  
  std::cout << "Pico Chess, Alpha version..." << std::endl;

  PicoChess::ChessEngine my_little_engine;
  PicoStreamPlayer::Play(&my_little_engine);
  
  return 0;
}

