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

void GuiStartup() {
  System_Init(); //System initialize, configure serial port and SPI interface...
  SD_Init(); 

  LCD_Init(lcd_scan_dir,800); // Initialize LCD panel,
                              //   confirm the scan mode and the brightness
    
  TP_Init(lcd_scan_dir);      // Initialize touch panel

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
//***********************************************************************

#define MENU_ORIGIN_X   0
#define MENU_ORIGIN_Y   0
#define MENU_TEXT_WIDTH 60

#define MENU_SELECT_X  MENU_ORIGIN_X + MENU_TEXT_WIDTH
#define MENU_SELECT_Y  MENU_ORIGIN_Y
#define MENU_SELECT_X_EXTENT MENU_SELECT_X + 16
#define MENU_SELECT_Y_EXTENT MENU_SELECT_Y + 16

#define BOARD_ORIGIN_X 8
#define BOARD_ORIGIN_Y MENU_SELECT_Y_EXTENT + 20
#define SQUARE_SIZE    28

#define SQUARE_COLOR_LIGHT YELLOW
#define SQUARE_COLOR_DARK  BROWN

#define BORDER_X BOARD_ORIGIN_X - 1
#define BORDER_Y BOARD_ORIGIN_Y - 1
#define BORDER_EXTENT_X BORDER_X + (SQUARE_SIZE * 8) + 1
#define BORDER_EXTENT_Y BORDER_Y + (SQUARE_SIZE * 8) + 1

#define BORDER_COLOR WHITE

#define STATUS_X BORDER_X
#define STATUS_Y BORDER_EXTENT_Y + 10
#define STATUS_EXTENT_X BORDER_EXTENT_X
#define STATUS_EXTENT_Y STATUS_Y + 32
#define STATUS_COLOR BORDER_COLOR

int OptionsSelected(int touch_x, int touch_y) {
  return ( (touch_x > MENU_SELECT_X) && (touch_x < MENU_SELECT_X_EXTENT) &&
	   (touch_y > MENU_SELECT_Y) && (touch_y < MENU_SELECT_Y_EXTENT) );
}

struct square_coords {
  unsigned char upper_left_x;
  unsigned char upper_left_y;
  unsigned char lower_right_x;
  unsigned char lower_right_y;
};

struct square_coords board_coords[8][8];

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

void DrawStatusBorder() {
  GUI_DrawRectangle(STATUS_X, STATUS_Y, STATUS_EXTENT_X, STATUS_EXTENT_Y,
		    STATUS_COLOR, DRAW_EMPTY, DOT_PIXEL_1X1);
}

// display text status below game board. Do multi-line comments work???

void DisplayStatus(char *the_status) {
}

void DisplayGameBoard() {
  sFONT* TP_Font = &Font16;
  
  GUI_DisString_EN(MENU_ORIGIN_X, MENU_ORIGIN_Y, "Options", TP_Font, BLACK, WHITE);

  int square_color = SQUARE_COLOR_DARK;
  
  for (int i = 7; i >= 0; i--) {
    for (int j = 0; j < 8; j++) {
      int this_square_x = BOARD_ORIGIN_X + i * SQUARE_SIZE;
      int this_square_y = BOARD_ORIGIN_Y + j * SQUARE_SIZE;
      
      GUI_DrawRectangle(this_square_x, this_square_y,
			this_square_x + SQUARE_SIZE, this_square_y + SQUARE_SIZE,
			square_color, DRAW_FULL, DOT_PIXEL_1X1);

      square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
      
      board_coords[i][j].upper_left_x  = this_square_x;
      board_coords[i][j].upper_left_y  = this_square_y;
      board_coords[i][j].lower_right_x = this_square_x + SQUARE_SIZE;
      board_coords[i][j].lower_right_y = this_square_y + SQUARE_SIZE;
    }
    
    square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;

  }

  GUI_DrawRectangle(BORDER_X, BORDER_Y, BORDER_EXTENT_X, BORDER_EXTENT_Y,
		    BORDER_COLOR, DRAW_EMPTY, DOT_PIXEL_1X1);

  DrawStatusBorder();
}


//***********************************************************************
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

#define BMP_BACKGROUND_COLOR_DARK 48192
#define DESIRED_BACKGROUND_COLOR_DARK SQUARE_COLOR_DARK

// translate bmp color of 24 bits to uint_16...

#define RGB24TORGB16(R,G,B) ((R>>3)<<11)|((G>>2)<<5)|(B>>3)

//#define DEBUG 1

static uint8_t tbuf[1024]; 

int ReadChessPieceBMP(uint16_t *image_buffer, const char *bmp_file) {
#ifdef DEBUG
  printf("[ReadChessPieceBMP] entered...\n");
#endif

  if (image_buffer == NULL) {
    printf("NULL IMAGE BUFFER???\n");
    return -1;
  }
  
  FIL file1; 
  UINT BytesRead;
  
  uint32_t size;      // bitmap size
  uint32_t index;     //   "    data address offset
  uint32_t width;     //   "    width
  uint32_t height;    //   "    height
  uint32_t bit_pixel; // bits per pixel
  
  f_open(&file1, bmp_file, FA_READ);	
  f_read(&file1, tbuf, BMP_HEADER_SIZE, &BytesRead);

  /* Read bitmap size */
  size = *(uint16_t *) &tbuf[2] | ( (*(uint16_t *) &tbuf[4]) << 16 );
  /* Get bitmap data address offset */
  index = *(uint16_t *) &tbuf[10] | ( (*(uint16_t *) &tbuf[12]) << 16 );
  /* Read bitmap width */
  width = *(uint16_t *) &tbuf[18] | ( (*(uint16_t *) &tbuf[20]) << 16 );
  /* Read bitmap height */
  height = *(uint16_t *) &tbuf[22] | ( (*(uint16_t *) &tbuf[24]) << 16 );
  /* Read bits per pixel */
  bit_pixel = *(uint16_t *) &tbuf[28];
  
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
  // and thus no need (gasp!) to close the file before 'reopening')
  
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

	uint16_t this_pixel_color = RGB24TORGB16(tbuf[k + 2],tbuf[k + 1],tbuf[k]);

	if (this_pixel_color == BMP_BACKGROUND_COLOR_LIGHT)
	  this_pixel_color = SQUARE_COLOR_LIGHT;
	else if (this_pixel_color == BMP_BACKGROUND_COLOR_DARK)
	  this_pixel_color = SQUARE_COLOR_DARK;
	
	image_buffer[ i * height + j] = this_pixel_color;

#ifdef DEBUG
	// lazy way of getting background color in prep for color matching above...
	if (background_color == 0) {
	  background_color = this_pixel_color; 
	  printf("Background color: %d\n",background_color);
	}
#endif
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
  
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "kdd30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "qdd30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "bdd30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "ndd30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "rdd30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "pdd30.bmp")) return -1;

  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "kld30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "qld30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "bld30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "nld30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "rld30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "pld30.bmp")) return -1;
  
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "kdl30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "qdl30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "bdl30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "ndl30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "rdl30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "pdl30.bmp")) return -1;

  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "kll30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "qll30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "bll30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "nll30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "rll30.bmp")) return -1;
  if (ReadChessPieceBMP((uint16_t *) piece_icons_dd.king, "pll30.bmp")) return -1;

#ifdef DEBUG
  printf("Chess piece images loaded successfully!\n");
#endif

  return 0;
}



