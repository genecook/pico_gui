#include <stdio.h>
#include <iostream>
#include <string>
#include "pico/stdlib.h"

#include <chess.h>

//*****************************************************************************
// supply 'pico-chess' stream player get_next_token, to_xboard functions...
//*****************************************************************************

extern "C" {
  void DisplayStatus(const char *the_status);
  void ReadScreenTouch(int *x, int *y);
  int  SquareSelected(int *row, int *column, int touch_x, int touch_y);
  void MoveChessPiece(const char *move);
  void DisplayToOptions(const char *the_status);
  void RowColumnToNotation(char *rank,char *file,int row,int column);
  void HilightSquare(int row, int column,int push);
  void DeHiLiteSquare(int row, int column,int pop);
}

namespace PicoStreamPlayer {
  // "new"  - new game
  // "usermove" - next token is move 
  static char move_str[80];

  enum { STARTUP, WAITING, MOVE_SEQUENCE_STARTED, HAVE_NEXT_MOVE };
  
  static int move_state = STARTUP;

  void get_square_selection(int *row, int *column) {
    int touch_x,touch_y;  
    while(1) {
      ReadScreenTouch(&touch_x, &touch_y);
      if (SquareSelected(row,column,touch_x,touch_y))
	break;
    }
    HilightSquare(*row,*column,1);    
  }
  
  void debug_wait(const char *prompt) {
    DisplayStatus(prompt);
    int rowX,columnX;
    get_square_selection(&rowX,&columnX);
  }
  
  void get_next_token(std::string &next_token) {
    /*
      int touch_x,touch_y;  
      int row,column;

      ReadScreenTouch(&touch_x, &touch_y);

      // process options request...

      if (OptionsSelected(touch_x, touch_y)) {
        DisplayStatus("No options yet.");
	return;
      }
*/
      
      char xbuf[20];
      switch(move_state) {
        case STARTUP:        // inform engine that "xboard" is connected...
	                     next_token = "xboard";
	                     move_state = WAITING;
                             return;
	                     break;
        case WAITING:        // start move sequence by returning 'usermove' token...
                             next_token = "usermove";
                             move_str[0] = '\0';
	                     move_state = MOVE_SEQUENCE_STARTED;
			     DisplayStatus(">>> usermove        ");
                             return;
	                     break;
        case MOVE_SEQUENCE_STARTED:
	                     // fall thru to get next move...
	                     DisplayStatus(">>> get next move   ");
	                     break;
        case HAVE_NEXT_MOVE: // have a 'next' move...
	                     //next_token = move_str;
	                     move_str[0] = '\0';
	                     move_state = WAITING;
			     sprintf(xbuf,">>> '%s'",next_token.c_str());
			     DisplayStatus(xbuf);
		             /*
			     {
                              int rowX,columnX;
                              get_square_selection(&rowX,&columnX);
			     }
			     */
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
      
      DeHiLiteSquare(-1,-1,1);
    
      // move should be validated BEFORE display is updated!

      MoveChessPiece(move_str);
      
      move_state = HAVE_NEXT_MOVE;
      /*
      int rowX = dest_row;
      int columnX = dest_column;
      while( (rowX == dest_row) && (columnX == dest_column) )  {
	get_square_selection(&rowX,&columnX);
      }
      */
      next_token = move_str;
  }

  // comments start with '#' - ignore
  // access comments with prefix '# BBB"
  // "move" + engine_move

  void to_xboard(std::string tbuf) {
    if (tbuf.size() > 18)
      DisplayToOptions(tbuf.substr(0,17).c_str());
    else
      DisplayToOptions(tbuf.c_str());
    //debug_wait(tbuf.c_str());

    size_t found = tbuf.find("\nmove ");
    if (found != std::string::npos) {
      MoveChessPiece((tbuf.substr(found + 6,4)).c_str());
    } else if (tbuf[0] == '#') {
      // just a comment...
    }
      else if (tbuf.substr(0,4) == "move") {
      // something else?...
    }
  }
}
