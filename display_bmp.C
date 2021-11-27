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
  void DrawChessPiecesNewGame();
  void ReadScreenTouch(int *x, int *y);
  int  SquareSelected(int *row, int *column, int touch_x, int touch_y);
  void RowColumnToNotation(char *rank,char *file,int row,int column);
  void MoveChessPiece(const char *move);
}

namespace PicoStreamPlayer {
  int Play(PicoChess::ChessEngine *the_engine);

  // "new"  - new game
  // "usermove" - next token is move 
  static char move_str[80] = {};

  enum { WAITING, MOVE_SEQUENCE_STARTED, HAVE_NEXT_MOVE };
  
  static int move_state = WAITING;

  void get_square_selection(int *row, int *column) {
    int touch_x,touch_y;  
    while(1) {
      ReadScreenTouch(&touch_x, &touch_y);
      if (SquareSelected(row,column,touch_x,touch_y))
	break;
    }
  }
  
  void get_next_token(std::string &next_token) {
      int touch_x,touch_y;  
      int row,column;

      ReadScreenTouch(&touch_x, &touch_y);

      // process options request...
      if (OptionsSelected(touch_x, touch_y)) {
        DisplayStatus("No options yet.");
	return;
      }

      switch(move_state) {
        case WAITING:        // start move sequence by returning 'usermove' token...
                             strcpy(move_str,"usermove");
                             move_state = MOVE_SEQUENCE_STARTED;
			     DisplayStatus(">>> usermove        ");
                             return;
	                     break;
        case MOVE_SEQUENCE_STARTED:
	                     // fall thru to get next move...
	                     DisplayStatus(">>> get next move   ");
	                     break;
        case HAVE_NEXT_MOVE: // have a 'next' move...
	                     next_token = move_str;
	                     move_str[0] = '\0';
	                     move_state = WAITING;
			     DisplayStatus(">>> move returned   ");
	                     return;
	                     break;
        default: break;
      }
      
      // get next move...
      
      char src_rank,src_file,dest_rank,dest_file;

      int src_row, src_column;
      get_square_selection(&src_row,&src_column);
            
      RowColumnToNotation(&src_rank,&src_file,src_row,src_column);

      char tbuf[128];
      sprintf(tbuf,">>> %c%c            ",src_file,src_rank);
      DisplayStatus(tbuf);

      int dest_row = src_row;
      int dest_column = src_column;
      
      while( (dest_row == src_row) && (dest_column == src_column) )  {
	get_square_selection(&dest_row,&dest_column);
      }
      
      RowColumnToNotation(&dest_rank,&dest_file,dest_row,dest_column);

      sprintf(move_str,"%c%c%c%c",src_file,src_rank,dest_file,dest_rank);

      sprintf(tbuf,">>> %s          ",move_str);
      DisplayStatus(tbuf);
      
      // move should be validated BEFORE display is updated!

      MoveChessPiece(move_str);
      
      move_state = HAVE_NEXT_MOVE;

      int rowX = dest_row;
      int columnX = dest_column;
      while( (rowX == dest_row) && (columnX == dest_column) )  {
	get_square_selection(&rowX,&columnX);
      }
  }

  // comments start with '#' - ignore
  // access comments with prefix '# BBB"
  // "move" + engine_move
  
  void to_xboard(std::string tbuf) {
    DisplayStatus(tbuf.c_str());
    
    if (tbuf[0] == '#') {
      // just a comment...
    } else if (tbuf == "move") {
      // update gui state...
      
    }

    int rowX,columnX;
    get_square_selection(&rowX,&columnX);
  }
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
    DrawChessPiecesNewGame();

    Wait(3000);

    stdio_init_all(); // don't need???
    
    std::cout << "Display BMP formatted image..." << std::endl;
    /*
    while(1) {
      int touch_x,touch_y;  
      ReadScreenTouch(&touch_x, &touch_y);      
    }
    */
    PicoChess::ChessEngine my_little_engine;
  
    PicoStreamPlayer::Play(&my_little_engine);

    return 0;
}



    
