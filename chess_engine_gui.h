#ifndef __CHESS_ENGINE_GUI__

  void GuiStartup();
  void DisplayImage(char *);
  void Wait(uint time_in_milliseconds);
  void ClearScreen();
  void ReadScreenTouch(int *x, int *y);
  void NewGame();
  
  void DisplayGameBoard();
  void DisplayStatus(const char *the_status);
  void DisplayToOptions(const char *the_status);
  
  int  LoadChessPieceImages();
  void DrawChessPiecesNewGame();

  enum { FOR_READ, FOR_WRITE };

  int  OpenGameFile(int for_write);
  int  CloseGameFile();
  int  WriteToGameFile(const char *tbuf);
  int  ReadFromGameFile(char *tbuf);
  int  FileRecordSize();

  int  SquareSelected(int *row, int *column, int touch_x, int touch_y);
  void MoveChessPiece(const char *move);
  void PlaceChessPieceCmd(const char *move);
  int  KingsMove(const char *move);
  int  PawnsMove(const char *move);
  void PromotePawn(const char *move);
  void RowColumnToNotation(char *rank,char *file,int row,int column);
  
  void HilightSquare(int row, int column,int push);
  void DeHiLiteSquare(int row, int column,int pop);
  
  void PlaceOptionsIcons();
  int  OptionSelected(int *option_index, int touch_x, int touch_y);
  int  ConfirmOption(int option_index);
  void ClearSelectedOption(int option_index);

typedef enum {
	      SAVE_GAME=0,
	      RESTORE_GAME,
	      PLAY_LEVEL,
	      CHANGE_SIDES,
	      UNDO_MOVE,
	      NEW_GAME,
	      SQUARE_SELECTED,
	      NO_SELECTION = -1
             }
              OPTIONS;

#endif
#define __CHESS_ENGINE_GUI__

