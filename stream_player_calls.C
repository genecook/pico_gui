#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include "pico/stdlib.h"

#include <chess.h>
#include <chess_engine_gui.h>

//*****************************************************************************
// supply 'pico-chess' stream player get_next_token, to_xboard functions...
//*****************************************************************************

namespace PicoStreamPlayer {

  //************************************************************************
  // read from touch panel, see if somthing interesting has been selected...
  //************************************************************************
  
  int get_next_selection(int *row, int *column) {
    int touch_x,touch_y;

    int selection_index = NO_SELECTION;
    
    ReadScreenTouch(&touch_x, &touch_y);
      
    int option_index;
      
    if (OptionSelected(&option_index, touch_x, touch_y)) {
      if (ConfirmOption(option_index)) {
        DisplayStatus("Option confirmed.");
	selection_index = option_index;
      } else
        DisplayStatus("Selected aborted.");
      ClearSelectedOption(option_index);
    }
    else if (SquareSelected(row,column,touch_x,touch_y)) {
      HilightSquare(*row,*column,1);    
      selection_index = SQUARE_SELECTED;
    }
      
    return selection_index;
  }

  //***********************************************************************
  // have the src/dest squares for a move, encode in standard notation...
  //***********************************************************************
  
  void encode_echo_move(char *move_str,int src_row,int src_column,int dest_row,int dest_column) {
      char src_rank,src_file,dest_rank,dest_file;
      RowColumnToNotation(&src_rank,&src_file,src_row,src_column);
      RowColumnToNotation(&dest_rank,&dest_file,dest_row,dest_column);

      sprintf(move_str,"%c%c%c%c",src_file,src_rank,dest_file,dest_rank);

      char tbuf[128];
      sprintf(tbuf,"users move: %s",move_str);
      DisplayStatus(tbuf);
      
      DeHiLiteSquare(-1,-1,1);    
  }
  
  //***************************************************************************
  // Invoked by the chess engine, this routine gets next move or directive
  //   from gui...
  //*************************************************************************** 

  enum { STARTUP, WAITING, MOVE_IN_PROGRESS, PROCESSING_MOVE,
	 PROCESSING_SAVE_GAME, PROCESSING_RESTORE_GAME, PROCESSING_PLAY_LEVEL,
	 PROCESSING_CHANGE_SIDES, PROCESSING_UNDO, PROCESSING_NEW_GAME
       }
        move_states;
  
  static int move_state = STARTUP; // current move state

  std::queue<std::string> token_queue;

  bool replay_queue_empty();
  void undo_last_move();
  void save_for_replay(std::string &next_token);
  void flush_replay_queue();
  void save_game();
  void restore_game();
  
  void get_next_token(std::string &next_token) {
    // process any queued up tokens first...
    
    if (!token_queue.empty()) {
      next_token = token_queue.front();
      token_queue.pop();
      save_for_replay(next_token);
      return;
    }

    // no tokens. fine, lets get some...
    
    int next_selection,start_row,start_column,end_row,end_column;
    char move_str[80];	

    // loop 'til there are tokens queued up...
    
    while(token_queue.empty()) { 
      switch(move_state) {
        case STARTUP:           // inform engine that "xboard" is connected...
	                        token_queue.push("xboard");
				token_queue.push("placepieces");
	                        move_state = WAITING;
	                        break;
			     
        case WAITING:           // waiting on user...
	                        next_selection = get_next_selection(&start_row,&start_column);

				switch(next_selection) {
				  case SQUARE_SELECTED: move_state = MOVE_IN_PROGRESS;        break;
			          case SAVE_GAME:       move_state = PROCESSING_SAVE_GAME;    break;
			          case RESTORE_GAME:    move_state = PROCESSING_RESTORE_GAME; break;
			          case PLAY_LEVEL:      move_state = PROCESSING_PLAY_LEVEL;   break;
			          case CHANGE_SIDES:    move_state = PROCESSING_CHANGE_SIDES; break;
			          case UNDO_MOVE:       move_state = PROCESSING_UNDO;         break;
			          case NEW_GAME:        move_state = PROCESSING_NEW_GAME;     break;
			          default: break;
			        }
                                break;
				
        case PROCESSING_SAVE_GAME:    // save game to flash...
	                              save_game();
	                              move_state = WAITING;
	                              break;
				      
        case PROCESSING_RESTORE_GAME: // restore previously saved game from flash...
	                              NewGame();
				      while(!token_queue.empty()) token_queue.pop();
                                      flush_replay_queue();
	                              restore_game();
	                              move_state = WAITING;
	                              break;
				      
        case PROCESSING_PLAY_LEVEL:   // change game engine play level...
	                              token_queue.push("togglelevels");
	                              move_state = WAITING;
	                              break;
				      
        case PROCESSING_CHANGE_SIDES: // change sides...
	                              DisplayStatus("Change sides? Nyet.");
	                              token_queue.push("changesides");
	                              token_queue.push("go");
				      token_queue.push("showside");
	                              move_state = WAITING;
	                              break;
				      
        case PROCESSING_UNDO:         // undo last move...
	                              if (replay_queue_empty()) {
					// game just started, ie, nothing to undo...
				        DisplayStatus("Nothing to undo...");
				        move_state = WAITING;
	                                break;
				      }
				      DisplayStatus("Undo last move...");
				      // flush token queue...
				      NewGame();
				      undo_last_move();
	                              move_state = WAITING;
	                              break;

				      
        case PROCESSING_NEW_GAME:     // setup new game...
	                              NewGame();
				      while(!token_queue.empty()) token_queue.pop();
                                      flush_replay_queue();
	                              token_queue.push("new");
				      token_queue.push("placepieces");
                                      move_state = WAITING;
	                              break;

        case MOVE_IN_PROGRESS:        // waiting 'til move completes or aborts...
	                              next_selection = get_next_selection(&end_row,&end_column);
			              switch(next_selection) {
			                case NO_SELECTION:
				          // waiting on dest square selection to complete move...
			                  break;
			              case SQUARE_SELECTED:
				          // make sure we have a new square selected...
				          if ( (end_row==start_row) && (end_column==start_column) )
				            break;
				          else
			                    move_state = PROCESSING_MOVE;
			                  break;
			              default:
				          // selection made that is NOT dest square? fine,
				          // clear state and continue...
				          DisplayStatus("Move aborted.");      
                                          DeHiLiteSquare(-1,-1,1);    
				          move_state = WAITING;
	                                  break;
			              }
			              break;
				
        case PROCESSING_MOVE:         // queue up next move. engine to validate same and update game board...
                                      encode_echo_move(move_str,start_row,start_column,end_row,end_column);
                                      token_queue.push("checkmove");
			              token_queue.push(move_str);
                                      move_state = WAITING;
			              break;

        default: break;
      }
    }
  }

  //***************************************************************************
  // replay/undo/save/restore all done via replay-queue...
  //*************************************************************************** 

  std::deque<std::string> replay_queue;

  void flush_replay_queue() {
     while(!replay_queue.empty()) replay_queue.pop_front();
  }

  bool replay_queue_empty() {
    return replay_queue.empty();
  }

  void save_game() {
    DisplayStatus("Saving game...");

    if (replay_queue.size() == 0) {
      DisplayStatus("No moves to save.");
      return;
    }
    
    if (OpenGameFile(FOR_WRITE) != 0) {
      DisplayStatus("Can't open game file?");
      return;
    }

    for (auto it = replay_queue.begin(); it != replay_queue.end(); it++) {
      if (WriteToGameFile((*it).c_str()) != 0) {
        DisplayStatus("Bad game file write?");
        return;
      }	
    }

    CloseGameFile();

    DisplayStatus("Game has been saved.");
  }

  void restore_game() {
    DisplayStatus("Reloading saved game...");

    OpenGameFile(FOR_READ);

    char tbuf[ FileRecordSize() ];

    while(ReadFromGameFile(tbuf) == 0) {
      token_queue.push(tbuf);
    }

    CloseGameFile();
    
    DisplayStatus("Game state restored.");
  }
  
  void save_for_replay(std::string &next_token) {
    // with current state of touch, this occurs pretty commonly...
    if (next_token == "# Invalid move") return;
    replay_queue.push_back(next_token);    
  }
  
  // undo last move - replay entire game up to but not including last move...

  void undo_last_move() {
    // flush token queue...
    while(!token_queue.empty()) token_queue.pop();
    token_queue.push("new");
    // copy/remove all replay queue elements to token queue, up to last element...
    while(!replay_queue.empty()) {
      std::string replay_element = replay_queue.front();
      replay_queue.pop_front();
      if (replay_queue.empty()) {
	// skip last move...
      } else if ( (replay_queue.size() == 1) && (replay_element == "checkmove") ) {
	// skip user move 'start' token...
      } else {
	token_queue.push(replay_element);
      }
    }
  }

  //***************************************************************************
  // update gui from chess engine...
  //
  // comments start with '#' - ignore
  // "move" + engine_move
  //
  // what we expect from engine:
  //   # BBB xboard
  //   # BBB OK move
  //   # checkmove e2e4
  //   # usermove e2e4
  //   # Engine moves...
  //   # BBB engine move made: # of levels: 3, number of moves evaluated: 20435\nmove g8f6
  //   # Invalid move
  //   move e8g8 - castle, black, kings side   -+
  //   move e8a8 -   "       "    queens side   +- in all cases, gui needs to recognize
  //   move e1g1 -   "     white  kings side    |   castling and update rooks position
  //   move e1a1 -   "       "    queens side  -+
  //***************************************************************************

  void check_for_castling(std::string &move) {
    // assuming king was piece to be moved...
    if      (move == "e8g8") MoveChessPiece("h8f8");
    else if (move == "e8c8") MoveChessPiece("a8d8");
    else if (move == "e1g1") MoveChessPiece("h1f1");
    else if (move == "e1c1") MoveChessPiece("a1d1");
  }

  void check_for_pawn_promotion(std::string &move) {
    // assuming pawn was piece to be moved...
    if ( (move[1] == '7') && (move[3] == '8') || (move[1] == '2') && (move[3] == '1') )
      PromotePawn(move.c_str());
  }
  
  bool check_for_game_over(std::string &outcome, std::string &possible_mating_move) {
    bool game_over = false;
    if (possible_mating_move.find("1-0 {Black mates}") != std::string::npos) {
	outcome = "Checkmate - Black!";
	game_over = true;
    } else if (possible_mating_move.find("0-1 {White mates}") != std::string::npos) {
      outcome = "Checkmate - White!";
      game_over = true;
    } else if (possible_mating_move.find("resign") != std::string::npos) {
      outcome = "Computer resigns!";
      game_over = true;
    } else if (possible_mating_move.find("1/2-1/2 {Draw by repetition}") != std::string::npos) {
      outcome = "Draw!";
      game_over = true;
    }
    return game_over;
  }

  std::string outcome; // for now, no way to restart game
  
  void to_xboard(std::string tbuf) {

    if (tbuf.substr(0,7) == "feature") {
      // ignore xboard 'feature' request...
      return;
    }

    size_t found = tbuf.find("\nmove ");

    if (found != std::string::npos) {
      // update game board with computers move...
      std::string cpu_move = tbuf.substr(found + 6,4); 
      bool king_to_move = KingsMove(cpu_move.c_str());
      bool pawn_to_move = PawnsMove(cpu_move.c_str());
      MoveChessPiece(cpu_move.c_str());
      if (king_to_move)
	check_for_castling(cpu_move);
      else if (pawn_to_move)
        check_for_pawn_promotion(cpu_move);	
      DisplayStatus(("cpu move: " + cpu_move).c_str());
      if (check_for_game_over(outcome,tbuf))
        DisplayStatus(outcome.c_str());
      return;
    }

    found = tbuf.find("checkmove ");

    if (found != std::string::npos) {
      // users move has been validated; update game board...
      std::string users_move = tbuf.substr(found + 10,4);
      bool king_to_move = KingsMove(users_move.c_str());
      bool pawn_to_move = PawnsMove(users_move.c_str());
      MoveChessPiece(users_move.c_str());
      if (king_to_move)
	check_for_castling(users_move);
      else if (pawn_to_move)
        check_for_pawn_promotion(users_move);	
      DisplayStatus(("user move: " + users_move).c_str()); 
      return;
    }

    found = tbuf.find("useriswhite");
    
    if (found != std::string::npos) {
      DisplayStatus("User plays white."); 
      return;
    }

    found = tbuf.find("userisblack");
    
    if (found != std::string::npos) {
      DisplayStatus("User plays black."); 
      return;
    }

    found = tbuf.find("placepiece ");
    
    if (found != std::string::npos) {
      std::string piece_to_place = tbuf.substr(found + 11,4); 
      PlaceChessPieceCmd(piece_to_place.c_str());
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
