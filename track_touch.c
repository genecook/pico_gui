#include "main.h"   //Examples
#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include <stdio.h>
#include "hardware/watchdog.h"
#include "pico/stdlib.h"

//****************************************************************************************
// display 'touch' coordinates...
//****************************************************************************************

void InitTouchPanel( LCD_SCAN_DIR Lcd_ScanDir );
void ReadTouch(POINT *x, POINT *y);

int main(void)
{
    System_Init(); //System intialize, configure serial port and SPI interface...
    SD_Init(); 

    LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT; //Set the scanmode 
    LCD_Init(lcd_scan_dir,800); // Initialize LCD panel,
                                //   confirm the scan mode and the brightness
    
    InitTouchPanel(lcd_scan_dir);  // Initialize touch panel

    GUI_Show();

    Driver_Delay_ms(3000); // seems to need good sized delay before continuing

    GUI_Clear(BLACK);

    stdio_init_all();
    printf("track touch...\n");
    
    // read touch, draw dots to track...

    char tbuf[128];
    sFONT* TP_Font = &Font16;
    
    while(1) {
      POINT x,y;
      ReadTouch(&x, &y);
      GUI_DrawPoint(x, y, WHITE, DOT_PIXEL_2X2 , DOT_FILL_AROUND);
      sprintf(tbuf,"x/y: %d/%d ",x,y);
      GUI_DisString_EN(10,10,tbuf,TP_Font, BLACK, WHITE);
    }
    
    return 0;
}


