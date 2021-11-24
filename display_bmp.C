#include <stdio.h>
#include <iostream>
#include <string>
#include "pico/stdlib.h"

#include <chess.h>

//*****************************************************************************
// example pico lcd program - display 'BMP' image...
//*****************************************************************************

extern "C" {
  void GuiStartup();
  void DisplayImage(char *);
  void DisplayGameBoard();
  void Wait(uint time_in_milliseconds);
  void ClearScreen();
  int  OptionsSelected(int touch_x, int touch_y);
  int  SquareSelected(int *row, int *column, int touch_x, int touch_y);
  void DisplayStatus(char *the_status);
  void my_LCD_Show_bmp(char *fname);
  int  LoadChessPieceImages();
}

namespace PicoStreamPlayer {
  int Play(PicoChess::ChessEngine *the_engine);
}

int main(void)
{
    GuiStartup();
    DisplayImage((char *) "chesbrd.bmp");
    Wait(3000);
    ClearScreen();
    DisplayGameBoard();
    
    my_LCD_Show_bmp((char *) "kdl30.bmp");

    LoadChessPieceImages();    
    Wait(3000);

    stdio_init_all(); // don't need???
    
    std::cout << "Display BMP formatted image..." << std::endl;

    PicoChess::ChessEngine my_little_engine;
  
    PicoStreamPlayer::Play(&my_little_engine);
  
    return 0;
}



    