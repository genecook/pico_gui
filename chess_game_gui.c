#include "main.h"

#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"

#include <stdio.h>

#include "hardware/watchdog.h"

#include "pico/stdlib.h"
#include "ff.h"
#include "fatfs_storage.h"

void InitTouchPanel( LCD_SCAN_DIR Lcd_ScanDir );
void ReadTouch(POINT *x, POINT *y);
void RowColumnToNotation(char *rank,char *file,int row,int column);

//*****************************************************************************
// LCD agnostic access to touch...
//*****************************************************************************

void ReadScreenTouch(int *x, int *y) {
  POINT px, py;
  ReadTouch(&px, &py);
  *x = px;
  *y = py;
}

//*****************************************************************************
// pico chess-game gui api...
//*****************************************************************************

LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT; // scanmode 
LCD_SCAN_DIR bmp_scan_dir = D2U_R2L;      // LCD type

//***********************************************************************
// similar to API LCD_Show_bmp call, only this abbreviated version
// displays only a single image...
//***********************************************************************

void DisplayImage(char *fname) {
  uint32_t bmplen = 0;
  
  if (Storage_CheckBitmapFile((const char*) fname, &bmplen)) {
    // either SD card not in or bmp file format incorrect...
    return;
  }

  // scan and display bmp image...
  LCD_SetGramScanWay( bmp_scan_dir );
  Storage_OpenReadFile(0, 0, (const char*) fname);

  // restore default scan...
  LCD_SetGramScanWay( lcd_scan_dir );
  DEV_Digital_Write(SD_CS_PIN,1);
}

//***********************************************************************
// sequence used to initialize LCD/touch panel...
//***********************************************************************

void GuiStartup() {
  System_Init(); //System initialize, configure serial port and SPI interface...
  SD_Init(); 

  LCD_Init(lcd_scan_dir,800); // Initialize LCD panel,
                              //   confirm the scan mode and the brightness
    
  InitTouchPanel(lcd_scan_dir); // Initialize touch panel

  GUI_Show();
}

//***********************************************************************
// some gui utilities...
//***********************************************************************

void Wait(uint time_in_milliseconds) {
  Driver_Delay_ms(3000);
}

void ClearScreen() {
  GUI_Clear(BLACK);
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

//#define SQUARE_COLOR_LIGHT YELLOW
//#define SQUARE_COLOR_DARK  BROWN

// background colors taken from chess piece bmp files:

#define SQUARE_COLOR_LIGHT 65529
#define SQUARE_COLOR_DARK  1472

#define BORDER_X BOARD_ORIGIN_X + 1
#define BORDER_Y BOARD_ORIGIN_Y + 1
#define BORDER_EXTENT_X BORDER_X + (SQUARE_SIZE * 8) - 2
#define BORDER_EXTENT_Y BORDER_Y + (SQUARE_SIZE * 8) - 1

#define BORDER_COLOR WHITE

#define STATUS_X BORDER_X
#define STATUS_Y BORDER_EXTENT_Y + 10
#define STATUS_EXTENT_X BORDER_EXTENT_X
#define STATUS_EXTENT_Y STATUS_Y + 32
#define STATUS_COLOR BORDER_COLOR
#define STATUS_TEXT_X STATUS_X + 8
#define STATUS_TEXT_Y STATUS_Y + 8
  
//***********************************************************************
// return true if options 'bar' touched...
//***********************************************************************

int OptionsSelected(int touch_x, int touch_y) {
  return ( (touch_x > MENU_SELECT_X) && (touch_x < MENU_SELECT_X_EXTENT) &&
	   (touch_y > MENU_SELECT_Y) && (touch_y < MENU_SELECT_Y_EXTENT) );
}

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
  GUI_DrawRectangle(STATUS_X, STATUS_Y, STATUS_EXTENT_X, STATUS_EXTENT_Y,
		    STATUS_COLOR, DRAW_EMPTY, DOT_PIXEL_1X1);
}

// display text status below game board. Do multi-line comments work???

void DisplayStatus(const char *the_status) {
  sFONT* TP_Font = &Font16;
  DrawStatusBorder();
  GUI_DisString_EN(STATUS_TEXT_X, STATUS_TEXT_Y, the_status, TP_Font, BLACK, WHITE);
}

void DisplayToOptions(const char *the_status) {
  sFONT* TP_Font = &Font16;
  //DrawStatusBorder();
  GUI_DisString_EN(MENU_ORIGIN_X, MENU_ORIGIN_Y, the_status, TP_Font, BLACK, WHITE);
}

//***********************************************************************
// draw the chess board, options and status bars (such as they are)...
//***********************************************************************

void DisplayGameBoard() {
  sFONT* TP_Font = &Font16;
  
  GUI_DisString_EN(MENU_ORIGIN_X, MENU_ORIGIN_Y, "Options", TP_Font, BLACK, WHITE);

  int square_color = SQUARE_COLOR_LIGHT;
  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int this_square_x = BOARD_ORIGIN_X + i * SQUARE_SIZE;
      int this_square_y = BOARD_ORIGIN_Y + j * SQUARE_SIZE;
      
      GUI_DrawRectangle(this_square_x, this_square_y,
			this_square_x + SQUARE_SIZE, this_square_y + SQUARE_SIZE,
			square_color, DRAW_FULL, DOT_PIXEL_1X1);

      // store away the coordinates for each chess board square...
      board_coords[i][j].upper_left_x  = this_square_x;
      board_coords[i][j].upper_left_y  = this_square_y;
      board_coords[i][j].lower_right_x = this_square_x + SQUARE_SIZE;
      board_coords[i][j].lower_right_y = this_square_y + SQUARE_SIZE;
      
      board_coords[i][j].square_color = (square_color == SQUARE_COLOR_DARK) ? DARK : LIGHT;

      // square colors alternate of course...
      square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
    }
    
    square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
  }

  GUI_DrawRectangle(BORDER_X, BORDER_Y, BORDER_EXTENT_X, BORDER_EXTENT_Y,
		    BORDER_COLOR, DRAW_EMPTY, DOT_PIXEL_1X1);

  DrawStatusBorder();
}

// after chess board is displayed (via above DisplayGameBoard function),
// individual board squares may be redrawn via this function...

void RedrawBoardSquare(int row, int column) {
  int square_color = (board_coords[row][column].square_color == LIGHT) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
  
  GUI_DrawRectangle(board_coords[row][column].upper_left_x,
		    board_coords[row][column].upper_left_y,
		    board_coords[row][column].upper_left_x + SQUARE_SIZE,
		    board_coords[row][column].upper_left_y + SQUARE_SIZE,
		    square_color,
		    DRAW_FULL, DOT_PIXEL_1X1);
}

//***********************************************************************
// lifted from vendor LCD/flash-access code. Modified so as to display
// only a single BMP-formated image, read from flash...
//***********************************************************************

uint32_t my_Storage_OpenReadFile(uint8_t Xpoz, uint16_t Ypoz, const char* BmpName);

void my_LCD_Show_bmp(char *fname) {
  System_Init(); //System initialize, configure serial port???
  SD_Init(); 

  LCD_Init(lcd_scan_dir,800); // Initialize LCD panel,
                              //   confirm the scan mode and the brightness
    
  TP_Init(lcd_scan_dir);      // Initialize touch panel

  GUI_Show();

  uint32_t bmplen = 0;
  
  if (Storage_CheckBitmapFile((const char*) fname, &bmplen)) {
    // either SD card not in or bmp file format incorrect...
    return;
  }

  // scan and display bmp image...
  LCD_SetGramScanWay( bmp_scan_dir );
  my_Storage_OpenReadFile(0, 0, (const char*) fname);

  // restore default scan...
  LCD_SetGramScanWay( lcd_scan_dir );
  DEV_Digital_Write(SD_CS_PIN,1);
}

//***********************************************************************
// ReadChessPieceBMP - read chess piece graphic image, formatted as bmp
//                     of 30 x 30 pixels, 24 colors per pixel...
//***********************************************************************

// chess piece bmp file format as 30x30 pixels, 24 colors per pixel...

#define BMP_HEADER_SIZE 30
#define NUM_PIXEL_ROWS  30
#define PIXELS_PER_ROW  30
#define BITS_PER_PIXEL  24

#define BYTES_PER_BMP_ROW 92  // 3 bytes per pixel times 30 pixels,
                              //   round up to four byte boundary

// imperically determined light/dark background colors in bmp;
// replace to match our chess board...

#define BMP_BACKGROUND_COLOR_LIGHT 65529
#define DESIRED_BACKGROUND_COLOR_LIGHT SQUARE_COLOR_LIGHT

#define BMP_BACKGROUND_COLOR_DARK 1472
#define DESIRED_BACKGROUND_COLOR_DARK SQUARE_COLOR_DARK

// translate bmp color of 24 bits to uint_16...

#define RGB24TORGB16(R,G,B) ((R>>3)<<11)|((G>>2)<<5)|(B>>3)

//#define DEBUG 1

static uint8_t tbuf[1024]; 

int ReadChessPieceBMP(uint16_t image_buffer[NUM_PIXEL_ROWS][PIXELS_PER_ROW], const char *bmp_file) {
#ifdef DEBUG
  printf("[ReadChessPieceBMP] entered...\n");
#endif

  if (image_buffer == NULL) {
    printf("NULL IMAGE BUFFER???\n");
    return -1;
  }
  
  FIL file1; 
  UINT BytesRead;
  
  f_open(&file1, bmp_file, FA_READ);	
  f_read(&file1, tbuf, BMP_HEADER_SIZE, &BytesRead);

  // endianness issues unless these parameters are read from the bmp header like so:
  uint32_t size      = *(uint16_t *) &tbuf[2]  | ( (*(uint16_t *) &tbuf[4]) << 16  ); // read bitmap size,
  uint32_t index     = *(uint16_t *) &tbuf[10] | ( (*(uint16_t *) &tbuf[12]) << 16 ); //  data address offset,
  uint32_t width     = *(uint16_t *) &tbuf[18] | ( (*(uint16_t *) &tbuf[20]) << 16 ); //  bitmap width,
  uint32_t height    = *(uint16_t *) &tbuf[22] | ( (*(uint16_t *) &tbuf[24]) << 16 ); //  bitmap height,
  uint32_t bit_pixel = *(uint16_t *) &tbuf[28];                                       //  bits per pixel
  
#ifdef DEBUG
  printf("file size      = %d \r\n", size);
  printf("file index     = %d \r\n", index);
  printf("file width     = %d \r\n", width);
  printf("file height    = %d \r\n", height);
  printf("bits per pixel = %d \r\n", bit_pixel);
#endif

  if (bit_pixel != BITS_PER_PIXEL) {
    printf("BITS PER PIXEL (%d) IS INCORRECT!\n", bit_pixel);
    f_close(&file1);
    return -1;
  }

  // use the index (offset to file data) to read past the
  // bmp file header...
  
  // (will ASSUME (from vendor supplied sample code) f_open 'resyncs'
  // and thus no need to close the file before 'reopening')
  
  f_open(&file1, bmp_file, FA_READ);
  f_read(&file1, tbuf, index, &BytesRead);

  if (BytesRead != index) {
    printf("BYTES READ TO INDEX (%d) INCORRECT?\n",BytesRead);
    f_close(&file1);
    return -1;
  }

  UINT background_color = 0;
  
  for (int i = 0; i < height; i++) {
     f_read(&file1, tbuf, BYTES_PER_BMP_ROW, &BytesRead);
     
     if (BytesRead != BYTES_PER_BMP_ROW) {
       printf("BYTES READ FOR ROW %d (%d) INCORRECT?\n",i,BytesRead);
       f_close(&file1);
       return -1;
     }
  
     for (int j = 0; j < width; j++) {
        int k = j * 3;

	uint16_t this_pixel_color = (uint16_t) (RGB24TORGB16(tbuf[k + 2],tbuf[k + 1],tbuf[k]));

#ifdef DEBUG
	// lazy way of getting background color in prep for color matching above...
	if (background_color == 0) {
	  background_color = this_pixel_color; 
	  printf("Background color: %u\n",background_color);
	}
#endif
	image_buffer[i][j] = this_pixel_color;
     }
  }

  f_close(&file1);

#ifdef DEBUG
  printf("[ReadChessPieceBMP] exited.\n");
#endif
  
  return 0; 
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

#ifdef DEBUG
  printf("Chess piece images loaded successfully!\n");
#endif

  return 0;
}

//***********************************************************************
// draw an 'image'...
//***********************************************************************

void DrawImage(int x, int y, uint16_t image[NUM_PIXEL_ROWS][PIXELS_PER_ROW],
	       int num_rows, int num_cols) {
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < num_cols; j++) {
       GUI_DrawPoint(x + j, y + i, image[i][j], DOT_PIXEL_DFT, DOT_STYLE_DFT);
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

  uint16_t (*image)[PIXELS_PER_ROW];
  
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


