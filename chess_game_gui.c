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
