#include <stdio.h>
#include <iostream>
#include <string>
#include "pico/stdlib.h"

#include <chess.h>
#include <chess_engine_gui.h>

extern "C" {
  void GuiStartup();
  void DisplayImage(char *);
  void DisplayGameBoard();
  void Wait(uint time_in_milliseconds);
  void ClearScreen();
  int  LoadChessPieceImages();
  void DrawChessPiecesNewGame();
  void PlaceOptionsIcons();
}

namespace PicoStreamPlayer {
  int Play(PicoChess::ChessEngine *the_engine);
}

int main() {
  GuiStartup();
  ClearScreen();
  DisplayGameBoard();
  LoadChessPieceImages();
  DrawChessPiecesNewGame();
  PlaceOptionsIcons();

  std::cout << "Pico Chess, Alpha version..." << std::endl;

  PicoChess::ChessEngine my_little_engine;
  PicoStreamPlayer::Play(&my_little_engine);
  
  return 0;
}

