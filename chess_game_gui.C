#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <stdexcept>

#include <chess_engine_gui.h>

// c functions:

extern "C" {
#include <lcd_touch_wrapper.h>
}

//***********************************************************************
// pico chess-game gui api...
//***********************************************************************

// C++ wrappers for gui utilities...

void ReadScreenTouch(int *x, int *y) {
  read_screen_touch(x,y);
}

void DisplayImage(char *fname) {
  display_image(fname);
}

void GuiStartup() {
  lcd_touch_startup();
  LoadChessPieceImages();
}

void Wait(uint time_in_milliseconds) {
  wait(3000);
}

void ClearScreen() {
  clear_screen();
}

//***********************************************************************
// setup a new game...
//***********************************************************************

void NewGame() {
  ClearScreen();
  DisplayGameBoard();
  //DrawChessPiecesNewGame();
  PlaceOptionsIcons();
}

//***********************************************************************
// screen parameters related to the display of the chess board, its
// options menu, and status bar...
//***********************************************************************

#define MENU_ORIGIN_X   0
#define MENU_ORIGIN_Y   0
#define MENU_TEXT_WIDTH 60

#define MENU_SELECT_X  MENU_ORIGIN_X + MENU_TEXT_WIDTH
#define MENU_SELECT_Y  MENU_ORIGIN_Y
#define MENU_SELECT_X_EXTENT MENU_SELECT_X + 16
#define MENU_SELECT_Y_EXTENT MENU_SELECT_Y + 16

#define BOARD_ORIGIN_X 0
#define BOARD_ORIGIN_Y MENU_SELECT_Y_EXTENT + 20
#define SQUARE_SIZE    30

// rough dimensions for button. see button code...
#define BUTTON_WIDTH    35
#define BUTTON_HEIGHT   20
#define BUTTON_OFFSET_X 5
#define BUTTON_OFFSET_Y 5
#define BUTTON_FONT     FONT_SIZE_12
#define BUTTON_COLOR    COLOR_WHITE

#define DIALOG_FONT     FONT_SIZE_12

//#define SQUARE_COLOR_LIGHT YELLOW
//#define SQUARE_COLOR_DARK  BROWN

// background colors taken from chess piece bmp files:

#define SQUARE_COLOR_LIGHT 65529
#define SQUARE_COLOR_DARK  1472

#define SQUARE_HIGHLIGHT_COLOR COLOR_RED

#define BORDER_X BOARD_ORIGIN_X + 1
#define BORDER_Y BOARD_ORIGIN_Y + 1
#define BORDER_EXTENT_X BORDER_X + (SQUARE_SIZE * 8) - 2
#define BORDER_EXTENT_Y BORDER_Y + (SQUARE_SIZE * 8) - 1

#define BORDER_COLOR COLOR_WHITE

#define STATUS_X BORDER_X
#define STATUS_Y BORDER_EXTENT_Y + 10
#define STATUS_EXTENT_X BORDER_EXTENT_X
#define STATUS_EXTENT_Y STATUS_Y + 32
#define STATUS_COLOR BORDER_COLOR
#define STATUS_TEXT_X STATUS_X + 8
#define STATUS_TEXT_Y STATUS_Y + 8
  
//***********************************************************************
// when the chess board is drawn, the coordinates for each square are
// stored in board_coords...
//***********************************************************************

enum { EMPTY = 0, KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN };
enum { LIGHT, DARK };      // chess board squares are either light or dark

enum { ADD_PIECE, REMOVE_PIECE };

// maintain enough game state to draw or update the game board during play.
// the chess engine is responsible for conveying move updates 'back' to the
// displayed game state...

struct square_coords {
  uint16_t upper_left_x;
  uint16_t upper_left_y;
  uint16_t lower_right_x;
  uint16_t lower_right_y;
  uint8_t  piece_type;
  uint8_t  piece_color;
  uint8_t  square_color;
};

struct square_coords board_coords[8][8];

void UpdateDisplayBoardState(int x,int y,uint8_t piece_type,uint8_t piece_color, int action) {
  if (action == ADD_PIECE) {
    board_coords[x][y].piece_type  = piece_type;
    board_coords[x][y].piece_color = piece_color;
  } else {
    board_coords[x][y].piece_type  = EMPTY;
    board_coords[x][y].piece_color = LIGHT; // board square is empty so color doesn't matter, still...
  }
}

void GetPieceInfo(uint8_t *piece_type,uint8_t *piece_color,int x, int y) {
  *piece_type  = board_coords[x][y].piece_type;
  *piece_color = board_coords[x][y].piece_color;  
}

// using board (square) coordinates, and a 'touch' point, identify which
// board square was accessed...

int SquareSelected(int *row, int *column, int touch_x, int touch_y) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      // make the touch determination inclusive, ie, the touch needs to be fully within
      // a square...
      if ( (touch_x > board_coords[i][j].upper_left_x) && (touch_x < board_coords[i][j].lower_right_x) &&
	   (touch_y > board_coords[i][j].upper_left_y) && (touch_y < board_coords[i][j].lower_right_y) ) {
	*row = i;
	*column = j;
	return 1;
      }
    }
  }

  return 0;
}

//***********************************************************************
// a status 'window' is maintained below the displayed chess board... 
//***********************************************************************

void DrawStatusBorder() {
  draw_rectangle(STATUS_X, STATUS_Y, STATUS_EXTENT_X, STATUS_EXTENT_Y,
		    STATUS_COLOR, NO_FILL);
}

// display text status below game board. Do multi-line comments work???

void DisplayStatus(const char *the_status) {
  DrawStatusBorder();
  display_string(STATUS_TEXT_X, STATUS_TEXT_Y, "                    ",
		 FONT_SIZE_16, COLOR_BLACK, COLOR_WHITE);
  char tbuf[20];
  strcpy(tbuf,"> ");
  if (strlen(the_status) > 17)
    strncat(tbuf,the_status,17);
  else
    strcat(tbuf,the_status);

  //sprintf(tbuf,"> %s",the_status);
  display_string(STATUS_TEXT_X, STATUS_TEXT_Y, tbuf,
		 FONT_SIZE_16, COLOR_BLACK, COLOR_WHITE);
}

//***********************************************************************
// draw the chess board, options and status bars (such as they are)...
//***********************************************************************

void DisplayGameBoard() {
  int square_color = SQUARE_COLOR_LIGHT;
  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int this_square_x = BOARD_ORIGIN_X + i * SQUARE_SIZE;
      int this_square_y = BOARD_ORIGIN_Y + j * SQUARE_SIZE;
      
      draw_rectangle(this_square_x, this_square_y,
		     this_square_x + SQUARE_SIZE, this_square_y + SQUARE_SIZE,
		     square_color, DO_FILL);

      // store away the coordinates for each chess board square...
      board_coords[i][j].upper_left_x  = this_square_x;
      board_coords[i][j].upper_left_y  = this_square_y;
      board_coords[i][j].lower_right_x = this_square_x + SQUARE_SIZE;
      board_coords[i][j].lower_right_y = this_square_y + SQUARE_SIZE;
      
      board_coords[i][j].square_color  = (square_color == SQUARE_COLOR_DARK) ? DARK : LIGHT;

      board_coords[i][j].piece_type    = EMPTY;
      board_coords[i][j].piece_color   = LIGHT;
    
      // square colors alternate within row...
      square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
    }
    // and from one row to the next...
    square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
  }
  
  draw_rectangle(BORDER_X, BORDER_Y, BORDER_EXTENT_X, BORDER_EXTENT_Y,
		 BORDER_COLOR, NO_FILL);

  DrawStatusBorder();
}

// after chess board is displayed (via above DisplayGameBoard function),
// individual board squares may be redrawn via this function...

uint16_t RealSquareColor(int row, int column) {
  return (board_coords[row][column].square_color == LIGHT) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
}

void RedrawBoardSquare(int row, int column) {
  int square_color = RealSquareColor(row,column);
  
  draw_rectangle(board_coords[row][column].upper_left_x,
		 board_coords[row][column].upper_left_y,
		 board_coords[row][column].lower_right_x,
		 board_coords[row][column].lower_right_y,
		 square_color,
		 DO_FILL);
}

//***********************************************************************
// to assist when selecting move components...
//***********************************************************************

#define MAX_HILIGHTS 5

struct square_hilite_coords {
  uint8_t row;
  uint8_t column;
};

struct square_hilite_coords hilited_squares[MAX_HILIGHTS];
static int hilite_index = -1;

void HilightSquare(int row, int column,int push) {
  if (push) {
    hilite_index++;
    if (hilite_index >= MAX_HILIGHTS)
      return;
    hilited_squares[hilite_index].row = row;
    hilited_squares[hilite_index].column = column;
  }
  
  draw_rectangle(board_coords[row][column].upper_left_x,
		 board_coords[row][column].upper_left_y,
		 board_coords[row][column].lower_right_x,
		 board_coords[row][column].lower_right_y,
		 SQUARE_HIGHLIGHT_COLOR,
		 NO_FILL);
}

void DeHiLiteSquare(int row, int column, int pop) {
  if (pop) {
    for (int i = 0; i < hilite_index; i++) {
      int hrow = hilited_squares[i].row;
      int hcol = hilited_squares[i].column;
      draw_rectangle(board_coords[hrow][hcol].upper_left_x,
		     board_coords[hrow][hcol].upper_left_y,
		     board_coords[hrow][hcol].lower_right_x,
		     board_coords[hrow][hcol].lower_right_y,
		     RealSquareColor(hrow,hcol),
		     NO_FILL);
    }
    hilite_index = -1;
  } else {
    draw_rectangle(board_coords[row][column].upper_left_x,
		   board_coords[row][column].upper_left_y,
		   board_coords[row][column].lower_right_x,
		   board_coords[row][column].lower_right_y,
		   RealSquareColor(row,column),
		   NO_FILL);
  }
}

//***********************************************************************
// simple-minded file i/o...
//***********************************************************************

int OpenGameFile(int for_write) { return open_game_file(for_write); }
int CloseGameFile() { return close_game_file(); }
int WriteToGameFile(const char *tbuf) { return write_to_game_file(tbuf); }
int ReadFromGameFile(char *tbuf) { return read_from_game_file(tbuf); }
int FileRecordSize() { return file_record_size(); };

//***********************************************************************
// ReadChessPieceBMP - read chess piece graphic image, formatted as bmp
//                     of 30 x 30 pixels, 24 colors per pixel...
//***********************************************************************

int ReadChessPieceBMP(uint16_t image_buffer[NUM_PIXEL_ROWS][PIXELS_PER_ROW], const char *bmp_file) {
  return read_bmp30x30(image_buffer,bmp_file);
}

//***********************************************************************
// manage chess piece images...
//
// to properly display light or dark chess pieces on light or dark
// board squares, need corresponding chess piece bmp's...
//***********************************************************************

struct chess_piece_images {
  uint16_t king[NUM_PIXEL_ROWS][PIXELS_PER_ROW];
  uint16_t queen[NUM_PIXEL_ROWS][PIXELS_PER_ROW];
  uint16_t bishop[NUM_PIXEL_ROWS][PIXELS_PER_ROW];
  uint16_t knight[NUM_PIXEL_ROWS][PIXELS_PER_ROW];
  uint16_t rook[NUM_PIXEL_ROWS][PIXELS_PER_ROW];
  uint16_t pawn[NUM_PIXEL_ROWS][PIXELS_PER_ROW];  
};

//                                             piece   background
//                                             color    color
//                                            -------  ----------
struct chess_piece_images piece_icons_dd;  //  dark      dark
struct chess_piece_images piece_icons_dl;  //  dark      light
struct chess_piece_images piece_icons_ld;  //  light     dark
struct chess_piece_images piece_icons_ll;  //  light     light

int LoadChessPieceImages() {
#ifdef DEBUG
  printf("Loading chess piece images...\n");
#endif
  
  if (ReadChessPieceBMP(piece_icons_dd.king,   "kdd30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dd.queen,  "qdd30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dd.bishop, "bdd30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dd.knight, "ndd30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dd.rook,   "rdd30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dd.pawn,   "pdd30.bmp")) return -1;

  if (ReadChessPieceBMP(piece_icons_ld.king,   "kld30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ld.queen,  "qld30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ld.bishop, "bld30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ld.knight, "nld30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ld.rook,   "rld30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ld.pawn,   "pld30.bmp")) return -1;
  
  if (ReadChessPieceBMP(piece_icons_dl.king,   "kdl30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dl.queen,  "qdl30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dl.bishop, "bdl30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dl.knight, "ndl30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dl.rook,   "rdl30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_dl.pawn,   "pdl30.bmp")) return -1;

  if (ReadChessPieceBMP(piece_icons_ll.king,   "kll30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ll.queen,  "qll30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ll.bishop, "bll30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ll.knight, "nll30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ll.rook,   "rll30.bmp")) return -1;
  if (ReadChessPieceBMP(piece_icons_ll.pawn,   "pll30.bmp")) return -1;

  return 0;
}

//***********************************************************************
// draw an 'image'...
//***********************************************************************

void DrawImage(int x, int y, uint16_t image[NUM_PIXEL_ROWS][PIXELS_PER_ROW],
	       int num_rows, int num_cols) {
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < num_cols; j++) {
       draw_point(x + j, y + i, image[i][j]);
    }
  }
}

//***********************************************************************
// translate chess board square row/column index into standard notation,
// and vice-versa...
//***********************************************************************

void RowColumnToNotation(char *rank,char *file,int row,int column) {
  char files[] = { 'a','b','c','d','e','f','g','h' };
  *file = files[row];
  char ranks[] = { '8','7','6','5','4','3','2','1' };
  *rank = ranks[column];
}

void NotationToRowColumn(int *row,int *column,char file,char rank) {
  switch(file) {
  case 'a': *row = 0; break;
  case 'b': *row = 1; break;
  case 'c': *row = 2; break;
  case 'd': *row = 3; break;
  case 'e': *row = 4; break;
  case 'f': *row = 5; break;
  case 'g': *row = 6; break;
  case 'h': *row = 7; break;
  default: break;
  }
  switch(rank) {
  case '1': *column = 7; break;
  case '2': *column = 6; break;
  case '3': *column = 5; break;
  case '4': *column = 4; break;
  case '5': *column = 3; break;
  case '6': *column = 2; break;
  case '7': *column = 1; break;
  case '8': *column = 0; break;
  default: break;
  }
}

// translate standard notation into board square coordinates...

void BoardToScreenCoords(int *x,int *y,char file,char rank) {
  int row, column;
  
  NotationToRowColumn(&row,&column,file,rank);

  *x = board_coords[row][column].upper_left_x;
  *y = board_coords[row][column].upper_left_y;  
}

// given chess board (square) position in standard notation,
// return square 'color' (light or dark)...

int SquareColor(char file, char rank) {
  int row, column;
  
  NotationToRowColumn(&row,&column,file,rank);
  return board_coords[row][column].square_color;
}

// remove piece from board, redraw board square...

void RemovePiece(char file, char rank) {
  int row, column;

  NotationToRowColumn(&row,&column,file,rank);

  UpdateDisplayBoardState(row,column,EMPTY,LIGHT,REMOVE_PIECE);
  RedrawBoardSquare(row, column);
}

//***********************************************************************
// draw chess piece (given its type, color, and its intended position)...
//***********************************************************************

void PlaceChessPiece(char file, char rank, int piece_index, int piece_color) {
  int x, y;
  BoardToScreenCoords(&x,&y,file,rank);
  
  uint16_t (*image)[PIXELS_PER_ROW] = NULL;
  
  switch((piece_color << 1) | SquareColor(file,rank)) {
  case (LIGHT << 1) | LIGHT:
    switch(piece_index) {
      case KING:   image = piece_icons_ll.king;    break;
      case QUEEN:  image = piece_icons_ll.queen;   break;
      case BISHOP: image = piece_icons_ll.bishop;  break;
      case KNIGHT: image = piece_icons_ll.knight;  break;
      case ROOK:   image = piece_icons_ll.rook;    break;
      case PAWN:   image = piece_icons_ll.pawn;    break;
      default: break;
    }
    break;
  case (LIGHT << 1) | DARK:
    switch(piece_index) {
      case KING:   image = piece_icons_ld.king;    break;
      case QUEEN:  image = piece_icons_ld.queen;   break;
      case BISHOP: image = piece_icons_ld.bishop;  break;
      case KNIGHT: image = piece_icons_ld.knight;  break;
      case ROOK:   image = piece_icons_ld.rook;    break;
      case PAWN:   image = piece_icons_ld.pawn;    break;
      default: break;
    }
    break;
  case (DARK << 1) | LIGHT:
    switch(piece_index) {
      case KING:   image = piece_icons_dl.king;    break;
      case QUEEN:  image = piece_icons_dl.queen;   break;
      case BISHOP: image = piece_icons_dl.bishop;  break;
      case KNIGHT: image = piece_icons_dl.knight;  break;
      case ROOK:   image = piece_icons_dl.rook;    break;
      case PAWN:   image = piece_icons_dl.pawn;    break;
      default: break;
    }
    break;
  case (DARK << 1) | DARK:
    switch(piece_index) {
      case KING:   image = piece_icons_dd.king;    break;
      case QUEEN:  image = piece_icons_dd.queen;   break;
      case BISHOP: image = piece_icons_dd.bishop;  break;
      case KNIGHT: image = piece_icons_dd.knight;  break;
      case ROOK:   image = piece_icons_dd.rook;    break;
      case PAWN:   image = piece_icons_dd.pawn;    break;
      default: break;
    }
    break;
  default: break;
  }

  if (image == NULL) {
    // what the ???
  } else     
    DrawImage(x,y,image,NUM_PIXEL_ROWS,PIXELS_PER_ROW);

  int row,column;
  NotationToRowColumn(&row,&column,file,rank);
  UpdateDisplayBoardState(row,column,piece_index,piece_color,ADD_PIECE);
}

//***********************************************************************
// place/draw chess pieces for 'new game'... 
//***********************************************************************

void DrawChessPiecesNewGame() {
  PlaceChessPiece('a', '1', ROOK,   LIGHT);
  PlaceChessPiece('b', '1', KNIGHT, LIGHT);
  PlaceChessPiece('c', '1', BISHOP, LIGHT);
  PlaceChessPiece('d', '1', QUEEN,  LIGHT);
  PlaceChessPiece('e', '1', KING,   LIGHT);
  PlaceChessPiece('f', '1', BISHOP, LIGHT);
  PlaceChessPiece('g', '1', KNIGHT, LIGHT);
  PlaceChessPiece('h', '1', ROOK,   LIGHT);

  PlaceChessPiece('a', '2', PAWN,   LIGHT);
  PlaceChessPiece('b', '2', PAWN,   LIGHT);
  PlaceChessPiece('c', '2', PAWN,   LIGHT);
  PlaceChessPiece('d', '2', PAWN,   LIGHT);
  PlaceChessPiece('e', '2', PAWN,   LIGHT);
  PlaceChessPiece('f', '2', PAWN,   LIGHT);
  PlaceChessPiece('g', '2', PAWN,   LIGHT);
  PlaceChessPiece('h', '2', PAWN,   LIGHT);

  PlaceChessPiece('a', '8', ROOK,   DARK);
  PlaceChessPiece('b', '8', KNIGHT, DARK);
  PlaceChessPiece('c', '8', BISHOP, DARK);
  PlaceChessPiece('d', '8', QUEEN,  DARK);
  PlaceChessPiece('e', '8', KING,   DARK);
  PlaceChessPiece('f', '8', BISHOP, DARK);
  PlaceChessPiece('g', '8', KNIGHT, DARK);
  PlaceChessPiece('h', '8', ROOK,   DARK);

  PlaceChessPiece('a', '7', PAWN,   DARK);
  PlaceChessPiece('b', '7', PAWN,   DARK);
  PlaceChessPiece('c', '7', PAWN,   DARK);
  PlaceChessPiece('d', '7', PAWN,   DARK);
  PlaceChessPiece('e', '7', PAWN,   DARK);
  PlaceChessPiece('f', '7', PAWN,   DARK);
  PlaceChessPiece('g', '7', PAWN,   DARK);
  PlaceChessPiece('h', '7', PAWN,   DARK);
}

//***********************************************************************
// does this move involve the king???
//***********************************************************************

int PieceType(const char *move) {
  char starting_file = move[0];
  char starting_rank = move[1];

  uint8_t piece_type,piece_color;

  int row,column;
  NotationToRowColumn(&row,&column,starting_file,starting_rank);
  GetPieceInfo(&piece_type,&piece_color,row,column);

  return piece_type;
}

int KingsMove(const char *move) {
  return (PieceType(move) == KING);
}

int PawnsMove(const char *move) {
  return (PieceType(move) == PAWN);
}

//***********************************************************************
// move chess piece - ASSUME move has been validated!!!
//***********************************************************************

void MoveChessPiece(const char *move) {
  char starting_file = move[0];
  char starting_rank = move[1];
  char end_file      = move[2];
  char end_rank      = move[3];

  uint8_t piece_type,piece_color;

  int row,column;
  NotationToRowColumn(&row,&column,starting_file,starting_rank);
  GetPieceInfo(&piece_type,&piece_color,row,column);

  if (piece_type == EMPTY) {
    // buggy code dude!!!
    char tbuf[20];
    sprintf(tbuf,"!!! %s !!!",move);
    DisplayStatus(tbuf);
    return;
  }
  
  RemovePiece(starting_file,starting_rank);
  PlaceChessPiece(end_file,end_rank,piece_type,piece_color);
}

// this function called to place pieces at start of game:
void PlaceChessPieceCmd(const char *move) {
  int piece_color = (move[0] == 'W') || (move[0] == 'w') ? LIGHT : DARK;
  int piece_type;
  switch(move[1]) {
    case 'p': case 'P': piece_type = PAWN;   break;
    case 'r': case 'R': piece_type = ROOK;   break;
    case 'n': case 'N': piece_type = KNIGHT; break;
    case 'b': case 'B': piece_type = BISHOP; break;
    case 'q': case 'Q': piece_type = QUEEN;  break;
    case 'k': case 'K': piece_type = KING;   break;
    default: break;
  }
  char file = move[2];
  char rank = move[3];
  int row,column;
  NotationToRowColumn(&row,&column,file,rank);
  PlaceChessPiece(file,rank,piece_type,piece_color);
}

void PromotePawn(const char *move) {
  char file = move[2];
  char rank = move[3];

  uint8_t piece_type,piece_color;

  int row,column;
  NotationToRowColumn(&row,&column,file,rank);
  GetPieceInfo(&piece_type,&piece_color,row,column);

  if (piece_type != PAWN) {
    // buggy code dude!!!
    char tbuf[20];
    sprintf(tbuf,"!!! %s !!!",move);
    DisplayStatus(tbuf);
    return;
  }
  
  // ASSUME: pawn always promotes to queen
  PlaceChessPiece(file,rank,QUEEN,piece_color);
}

//***********************************************************************
// buttons...
//***********************************************************************

// based on the lcd screen size can get as many as six buttons
// (yea, might as well be hard coded, sigh)

#define BUTTON_0_X MENU_ORIGIN_X + 2
#define BUTTON_0_Y MENU_ORIGIN_Y + 5

#define BUTTON_SPAN BUTTON_WIDTH + BUTTON_OFFSET_X

#define BUTTON_1_X BUTTON_0_X + BUTTON_SPAN 
#define BUTTON_1_Y BUTTON_0_Y

#define BUTTON_2_X BUTTON_1_X + BUTTON_SPAN
#define BUTTON_2_Y BUTTON_1_Y

#define BUTTON_3_X BUTTON_2_X + BUTTON_SPAN
#define BUTTON_3_Y BUTTON_2_Y

#define BUTTON_4_X BUTTON_3_X + BUTTON_SPAN - 2
#define BUTTON_4_Y BUTTON_3_Y

#define BUTTON_5_X BUTTON_4_X + BUTTON_SPAN - 2
#define BUTTON_5_Y BUTTON_4_Y

#define BUTTON_TEXT_OFFSET_X 5
#define BUTTON_TEXT_OFFSET_Y 5

void DrawButton(uint16_t origin_x, uint16_t origin_y, const char *button_text, int do_hilite) {
  uint16_t fill_option = do_hilite ? DO_FILL : NO_FILL;

  // 'erase', ie, write over previous button in case it had previously been hilited...
  draw_rectangle(origin_x, origin_y,
		 origin_x + BUTTON_WIDTH, origin_y + BUTTON_HEIGHT,
		 COLOR_BLACK, DO_FILL);
    
  draw_rectangle(origin_x, origin_y,
		 origin_x + BUTTON_WIDTH, origin_y + BUTTON_HEIGHT,
		 BUTTON_COLOR, fill_option);

  // fudge 'centering' the button text:
  uint16_t text_offset = strlen(button_text) < 4 ? 2 : 0;

  uint16_t font_background_color = do_hilite ? BUTTON_COLOR : COLOR_BLACK;
  uint16_t font_color = do_hilite ? COLOR_BLACK : BUTTON_COLOR;
  
  display_string(origin_x + BUTTON_TEXT_OFFSET_X + text_offset,
		 origin_y + BUTTON_TEXT_OFFSET_Y,
		 button_text,
		 BUTTON_FONT, font_background_color, font_color);
}

struct push_button {
  uint16_t  x;
  uint16_t  y;
  const char     *button_text;
  const char     *help_text;
};

struct push_button options[] = {
  { BUTTON_0_X, BUTTON_0_Y, "SAVE", "Save game?"         },
  { BUTTON_1_X, BUTTON_1_Y, "REST", "Restore game?"      },

  { BUTTON_2_X, BUTTON_2_Y, "LEVL", "Change play level?" },
  { BUTTON_3_X, BUTTON_3_Y, "SIDE", "Change sides?"      },

  { BUTTON_4_X, BUTTON_4_Y, "UNDO", "Undo last move?"    },
  { BUTTON_5_X, BUTTON_5_Y, "NEW",  "New game?"          }
};

// keep these in order!

#define SAVE_GAME    0
#define RESTORE_GAME 1
#define PLAY_LEVEL   2
#define SIDE         3 
#define UNDO_MOVE    4
#define NEW_GAME     5

#define NO_OPTION_SELECTED -1

#define NUM_OPTIONS 6

void PlaceOptionsIcons() {
  for (int i = 0; i < NUM_OPTIONS; i++) {
    DrawButton(options[i].x, options[i].y, options[i].button_text,0);
  }
}

//***********************************************************************
// return true if some option selected...
//***********************************************************************

int OptionSelected(int *option_index, int touch_x, int touch_y) {

  for (int i = 0; i < NUM_OPTIONS; i++) {
    uint16_t xs = options[i].x;
    uint16_t ys = options[i].y;
    uint16_t xe = xs + BUTTON_WIDTH;
    uint16_t ye = ys + BUTTON_HEIGHT;
    
    if ( (touch_x > xs) && (touch_x < xe) && (touch_y > ys) && (touch_y < ye) ) {
      *option_index = i;
      DrawButton(options[i].x,options[i].y,options[i].button_text,1);
      return 1;
    }
  }

  return 0;
}

void ClearSelectedOption(int option_index) {
  DrawButton(options[option_index].x,options[option_index].y,
	     options[option_index].button_text,0);
}

int ConfirmOption(int option_index) {
  // erase section in middle of display...
  uint16_t confirm_box_ulx = board_coords[1][3].upper_left_x;
  uint16_t confirm_box_uly = board_coords[1][3].upper_left_y;
  uint16_t confirm_box_lrx = board_coords[5][3].lower_right_x;
  uint16_t confirm_box_lry = board_coords[5][3].lower_right_y;

  draw_rectangle(confirm_box_ulx, confirm_box_uly,
		 confirm_box_lrx, confirm_box_lry,
		 COLOR_BLACK, DO_FILL);
  
  // post text for this option...put on dialog box 1st row

  DisplayStatus(options[option_index].help_text);
  
  uint16_t row1_text_box_ulx = confirm_box_ulx + 3;
  uint16_t row1_text_box_uly = confirm_box_uly + 5;
  
  display_string(row1_text_box_ulx, row1_text_box_uly,
		 "Confirm selection:",
		 DIALOG_FONT, COLOR_BLACK, COLOR_WHITE);

  uint16_t click_box_ulx = board_coords[5][3].upper_left_x + 10;
  uint16_t click_box_uly = board_coords[5][3].upper_left_y + 4;
  uint16_t click_box_lrx = click_box_ulx + 15;
  uint16_t click_box_lry = click_box_uly + 15;

  draw_rectangle(click_box_ulx, click_box_uly,
		 click_box_lrx, click_box_lry,
                 BORDER_COLOR, NO_FILL);

  // post confirm? + 'click-box', then wait for touch response...

  int option_confirmed = 0;

  // wait 'til some touch event...
  int x = -1, y = -1;
  ReadScreenTouch(&x, &y);

  // make 'accept' box larger than 'click' box, just to make it easier to 
  //  confirm selection...
  uint16_t accept_ulx = click_box_ulx - 10;
  uint16_t accept_uly = click_box_uly - 10;
  uint16_t accept_lrx = click_box_lrx + 10;
  uint16_t accept_lry = click_box_lry + 10;

  if ( (x >= accept_ulx) && (x < accept_lrx) && (y >= accept_uly) && (y < accept_lry) )
    option_confirmed = 1;
  
  // again, erase section in middle of display...
  
  draw_rectangle(confirm_box_ulx, confirm_box_uly,
		 confirm_box_lrx, confirm_box_lry,
		 COLOR_BLACK, DO_FILL);
  
  // redraw obscured portion of game board...

  for (int i = 1; i <= 5; i++) {
    for (int j = 3; j <= 3; j++) {
       RedrawBoardSquare(i,j);
       uint8_t piece_type,piece_color;
       GetPieceInfo(&piece_type,&piece_color,i,j);
       if (piece_type == EMPTY)
	 continue;
       char rank,file;
       RowColumnToNotation(&rank,&file,i,j);
       PlaceChessPiece(file,rank,piece_type,piece_color);
    }
  }

  // return indicator that option was indeed selected...

  return option_confirmed;
}




