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
      
      line_sweep(RIGHT_LOWER_CORNER,BLUE,line_style,dot_pixel);
      line_sweep(RIGHT_LOWER_CORNER,BLACK,line_style,dot_pixel);

      line_sweep(LEFT_LOWER_CORNER,GREEN,line_style,dot_pixel);
      line_sweep(LEFT_LOWER_CORNER,BLACK,line_style,dot_pixel);
      
	//Driver_Delay_ms(1000);
    }

    return 0;
}

void line_sweep_inner(int x_start,int y_start,int x_end,int y_end,
		     COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  GUI_DrawLine(x_start,y_start,x_end,y_end,color,line_style,dot_pixel);	
}

// # of pixels in X, Y direction:
#define X_PIXELS_MAX 240
#define Y_PIXELS_MAX 320

// last x, y addressible pixel:
#define LAST_X_PIXEL X_PIXELS_MAX - 1 
#define LAST_Y_PIXEL Y_PIXELS_MAX - 1

// DCT - transform 'logical' display coordinates (coordinates relative to corner of display)
//       into physical display coordinates (relative to upper left hand of pico lcd display)

void DCT(int *px, int *py, int lx, int ly, DISPLAY_CORNERS corner) {
  switch(corner) {
    case LEFT_UPPER_CORNER:  *px = lx;                *py = ly;                break;
    case RIGHT_UPPER_CORNER: *px = LAST_X_PIXEL - lx; *py = ly;                break;
    case RIGHT_LOWER_CORNER: *px = LAST_X_PIXEL - lx; *py = LAST_Y_PIXEL - ly; break;
    case LEFT_LOWER_CORNER:  *px = lx;                *py = LAST_Y_PIXEL - ly; break;
    default: break;
  }
}

void line_sweep(DISPLAY_CORNERS corner,COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  int x_incr = 20;
  int y_incr = 20;

  int px_start, py_start;
  DCT(&px_start,&py_start,0,0,corner);

  for (int x = 0; x < X_PIXELS_MAX; x += x_incr) {
     for (int y = 0; y < Y_PIXELS_MAX; y += y_incr) {
        int px_end, py_end;
        DCT(&px_end,&py_end,x,y,corner);
	line_sweep_inner(px_start,py_start,px_end,py_end,color,line_style,dot_pixel);	
     }
  }
}
