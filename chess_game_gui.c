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

      square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
    }
    square_color = (square_color == SQUARE_COLOR_DARK) ? SQUARE_COLOR_LIGHT : SQUARE_COLOR_DARK;
  }

  GUI_DrawRectangle(BORDER_X, BORDER_Y, BORDER_EXTENT_X, BORDER_EXTENT_Y,
		    BORDER_COLOR, DRAW_EMPTY, DOT_PIXEL_1X1);
  
}
