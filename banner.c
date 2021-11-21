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
// example pico lcd program - display 'BMP' image...
//*****************************************************************************

LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT; // scanmode 
LCD_SCAN_DIR bmp_scan_dir = D2U_R2L;      // LCD type

//***********************************************************************
// similar to API LCD_Show_bmp call, only this abbreviated version
// displays only a single image...
//***********************************************************************

void my_LCD_Show_bmp(char *fname) {
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

void ShowBanner(char *the_banner) {
  System_Init(); //System intialize, configure serial port and SPI interface...
  SD_Init(); 

  LCD_Init(lcd_scan_dir,800); // Initialize LCD panel,
                              //   confirm the scan mode and the brightness
    
  TP_Init(lcd_scan_dir);      // Initialize touch panel

  GUI_Show();

  // display program 'banner'...
 
  my_LCD_Show_bmp(the_banner);  
}
