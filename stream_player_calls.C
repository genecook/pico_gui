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
  int  OptionSelected(int *option_index, int touch_x, int touch_y);
  int  ConfirmOption(int option_index);
  void ClearSelectedOption(int option_index);
  void Wait(uint time_in_milliseconds);
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
      int option_index;
      if (OptionSelected(&option_index, touch_x, touch_y)) {
	if (ConfirmOption(option_index))
          DisplayStatus("Option confirmed.");
	else
          DisplayStatus("Selected aborted.");
        ClearSelectedOption(option_index);
      } 
      else if (SquareSelected(row,column,touch_x,touch_y))
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

      if (OptionSelected(touch_x, touch_y)) {
        DisplayStatus("No options yet.");
	Wait(2000);
	ClearSelectedOption();
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
			     //DisplayStatus(">>> usermove        ");
                             return;
	                     break;
        case MOVE_SEQUENCE_STARTED:
	                     // fall thru to get next move...
	                     //DisplayStatus(">>> get next move   ");
	                     break;
        case HAVE_NEXT_MOVE: // have a 'next' move...
	                     //next_token = move_str;
	                     move_str[0] = '\0';
	                     move_state = WAITING;
			     //sprintf(xbuf,">>> '%s'",next_token.c_str());
			     //DisplayStatus(xbuf);
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

      //char tbuf[128];
      //sprintf(tbuf,">>> %c%c            ",src_file,src_rank);
      //DisplayStatus(tbuf);

      int dest_row = src_row;
      int dest_column = src_column;
      
      while( (dest_row == src_row) && (dest_column == src_column) )  {
	get_square_selection(&dest_row,&dest_column);
      }
      
      RowColumnToNotation(&dest_rank,&dest_file,dest_row,dest_column);

      sprintf(move_str,"%c%c%c%c",src_file,src_rank,dest_file,dest_rank);

      char tbuf[128];
      sprintf(tbuf,"users move: %s",move_str);
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

    size_t found = tbuf.find("\nmove ");
    
    if (found != std::string::npos) {
      MoveChessPiece((tbuf.substr(found + 6,4)).c_str());
      DisplayStatus(("cpu move: " + tbuf.substr(found + 6,4)).c_str());
      return;
    }

    if (tbuf[0] == '#') {
      // just a comment...
      if (tbuf.size() > 18)
        DisplayStatus(tbuf.substr(0,17).c_str());
      else
        DisplayStatus(tbuf.c_str());
      return;
    }

    // something else?...

    if (tbuf.size() > 18)
      DisplayStatus(("?" + tbuf.substr(0,17)).c_str());
    else
      DisplayStatus(("?" + tbuf).c_str());
  }
}
