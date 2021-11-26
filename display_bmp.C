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
  void DisplayStatus(const char *the_status);
  void my_LCD_Show_bmp(char *fname);
  int  LoadChessPieceImages();
  void DrawChessPiece();
  void ReadScreenTouch(int *x, int *y);
  int  SquareSelected(int *row, int *column, int touch_x, int touch_y);
  void RowColumnToNotation(char *rank,char *file,int row,int column);
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
    
    //my_LCD_Show_bmp((char *) "kdd30.bmp");
    LoadChessPieceImages();
    DrawChessPiece();

    Wait(3000);

    stdio_init_all(); // don't need???
    
    std::cout << "Display BMP formatted image..." << std::endl;

    DisplayStatus(">>>");

    char tbuf[128];
    
    while(1) {
      int touch_x,touch_y;  
      ReadScreenTouch(&touch_x, &touch_y);
      
      int row,column;
      if (SquareSelected(&row,&column,touch_x,touch_y)) {
	char rank,file;
	RowColumnToNotation(&rank,&file,row,column);
        sprintf(tbuf," %c%c",file,rank);
      } else {
	sprintf(tbuf,"???");
      }
      
      DisplayStatus(tbuf);	
    }
    
    PicoChess::ChessEngine my_little_engine;
  
    PicoStreamPlayer::Play(&my_little_engine);

    return 0;
}



    
