#include "main.h"   //Examples
#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include <stdio.h>
#include "hardware/watchdog.h"
#include "pico/stdlib.h"

typedef enum { LEFT_UPPER_CORNER = 0,
	       RIGHT_UPPER_CORNER,
	       LEFT_LOWER_CORNER,
	       RIGHT_LOWER_CORNER
} DISPLAY_CORNERS;  

void line_sweep(DISPLAY_CORNERS corner, COLOR color, LINE_STYLE line_style, DOT_PIXEL dot_pixel);

int main(void)
{
    System_Init(); //System intialize, configure serial port and SPI interface...
    SD_Init(); 

    LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT; //Set the scanmode 
    LCD_Init(lcd_scan_dir,800); // Initialize LCD panel,
                                //   confirm the scan mode and the brightness
    
    TP_Init(lcd_scan_dir);      // Initialize touch panel

    GUI_Show();

    Driver_Delay_ms(3000); // seems to need good sized delay before continuing

    GUI_Clear(BLACK);

    COLOR color = WHITE;
    LINE_STYLE line_style = LINE_SOLID;
    DOT_PIXEL dot_pixel = DOT_PIXEL_1X1;

    while(1) {
        //GUI_Clear(BLACK);
	
      line_sweep(LEFT_UPPER_CORNER,WHITE,line_style,dot_pixel);
      line_sweep(LEFT_UPPER_CORNER,BLACK,line_style,dot_pixel);
      
      line_sweep(RIGHT_UPPER_CORNER,RED,line_style,dot_pixel);
      line_sweep(RIGHT_UPPER_CORNER,BLACK,line_style,dot_pixel);
      
      line_sweep(LEFT_LOWER_CORNER,GREEN,line_style,dot_pixel);
      line_sweep(LEFT_LOWER_CORNER,BLACK,line_style,dot_pixel);
      
      line_sweep(RIGHT_LOWER_CORNER,BLUE,line_style,dot_pixel);
      line_sweep(RIGHT_LOWER_CORNER,BLACK,line_style,dot_pixel);

	//Driver_Delay_ms(1000);
    }

    return 0;
}

void line_sweep_inner(int x_start,int y_start,int x_end,int y_end,
		     COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  GUI_DrawLine(x_start,y_start,x_end,y_end,color,line_style,dot_pixel);	
}

#define X_PIXELS_MAX 240
#define Y_PIXELS_MAX 320

void line_sweep(DISPLAY_CORNERS corner,COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  
  int x_min    = 0;
  int y_min    = 0;
  int x_max    = X_PIXELS_MAX;
  int y_max    = Y_PIXELS_MAX;
  int x_incr   = 20;
  int y_incr   = 20;

  switch(corner) {
    case 0: // origin is upper left hand corner...

            for (int x = x_min; x < x_max; x += x_incr) {
               for (int y = y_min; y < y_max; y += y_incr) {
		  line_sweep_inner(x_min,y_min,x,y,color,line_style,dot_pixel);	
               }
            }
            break;
  
    case 1: // origin is upper right hand corner...

            for (int x = x_max - 1; x >= x_min; x -= x_incr) {
               for (int y = y_min; y < y_max; y += y_incr) {
		  line_sweep_inner(x_max - 1,y_min,x,y,color,line_style,dot_pixel);	
               }
            }
            break;

    case 2: // origin  is lower right hand corner...

            for (int x = x_max - 1; x >= x_min; x -= x_incr) {
	       for (int y = y_max - 1; y >= y_min; y -= y_incr) {
	          line_sweep_inner(x_max - 1,y_max - 1,x,y,color,line_style,dot_pixel);	
               }
            }
            break;

    case 3: // origin is lower left hand corner...

            for (int x = x_min; x < x_max; x += x_incr) {
	       for (int y = y_max - 1; y >= y_min; y -= y_incr) {
  	          line_sweep_inner(x_min,y_min,x,y,color,line_style,dot_pixel);	
               }
            }
            break;

    default: break;
  }
}
