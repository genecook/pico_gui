#include <stdio.h>
#include <iostream>
#include <string>
#include "pico/stdlib.h"

#include <chess.h>

extern "C" {
  void GuiStartup();
  void DisplayImage(char *);
  void DisplayGameBoard();
  void Wait(uint time_in_milliseconds);
  void ClearScreen();
  int  OptionsSelected(int touch_x, int touch_y);
  int  SquareSelected(int *row, int *column, int touch_x, int touch_y);
  void DisplayStatus(char *the_status);
}

namespace PicoStreamPlayer {
  int Play(PicoChess::ChessEngine *the_engine);
}

int main() {
  GuiStartup();
  DisplayImage((char *) "chesbrd.bmp");
  Wait(3000);
  ClearScreen();
  DisplayGameBoard();
  stdio_init_all();

  std::cout << "Pico Chess, Alpha version..." << std::endl;
  
  PicoChess::ChessEngine my_little_engine;
  
  PicoStreamPlayer::Play(&my_little_engine);
  
  return 0;
}

