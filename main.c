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
// example pico lcd program - 'sweep' lines of different colors from corners...
//****************************************************************************************

// # of pixels in X, Y direction for Waveshare Pico-ResTouch-LCD-2.8:
#define X_PIXELS 240
#define Y_PIXELS 320

typedef enum { LEFT_UPPER_CORNER = 0,
	       RIGHT_UPPER_CORNER,
	       LEFT_LOWER_CORNER,
	       RIGHT_LOWER_CORNER
} DISPLAY_CORNERS;  

void screen_sweep(DISPLAY_CORNERS corner, COLOR color, LINE_STYLE line_style, DOT_PIXEL dot_pixel);

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
      // draw, then erase 'sweeping' set of lines from each corner of display...
      
      screen_sweep(LEFT_UPPER_CORNER,WHITE,line_style,dot_pixel);
      screen_sweep(LEFT_UPPER_CORNER,BLACK,line_style,dot_pixel);
      
      screen_sweep(RIGHT_UPPER_CORNER,RED,line_style,dot_pixel);
      screen_sweep(RIGHT_UPPER_CORNER,BLACK,line_style,dot_pixel);
      
      screen_sweep(RIGHT_LOWER_CORNER,BLUE,line_style,dot_pixel);
      screen_sweep(RIGHT_LOWER_CORNER,BLACK,line_style,dot_pixel);

      screen_sweep(LEFT_LOWER_CORNER,GREEN,line_style,dot_pixel);
      screen_sweep(LEFT_LOWER_CORNER,BLACK,line_style,dot_pixel);
    }

    return 0;
}

// DCT - transform 'logical' display coordinates (coordinates relative to corner of display)
//       into physical display coordinates (relative to upper left hand of pico lcd display)

void DCT(int *px, int *py, int lx, int ly, DISPLAY_CORNERS corner) {
  switch(corner) {
    case LEFT_UPPER_CORNER:  *px = lx;                *py = ly;                break;
    case RIGHT_UPPER_CORNER: *px = (X_PIXELS-1) - lx; *py = ly;                break;
    case RIGHT_LOWER_CORNER: *px = (X_PIXELS-1) - lx; *py = (Y_PIXELS-1) - ly; break;
    case LEFT_LOWER_CORNER:  *px = lx;                *py = (Y_PIXELS-1) - ly; break;
    default: break;
  }
}

// wrapper for GUI_DrawLine function; used with (commented out) delay
// during debug...

void draw_line(int x_start, int y_start, int x_end, int y_end,
		COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  GUI_DrawLine(x_start,y_start,x_end,y_end,color,line_style,dot_pixel);
  //Driver_Delay_ms(300);
}

// draw set of lines varying Y coordinate so as to 'sweep' along Y axis...

void side_sweep(DISPLAY_CORNERS corner, int px_start, int py_start, int y_incr,
		COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  switch(corner) {
    case LEFT_UPPER_CORNER:
    case RIGHT_LOWER_CORNER:      
      for (int y = 0; y < Y_PIXELS; y += y_incr) {
         int px_end, py_end;
         DCT(&px_end,&py_end,X_PIXELS - 1,y,corner);
         draw_line(px_start,py_start,px_end,py_end,color,line_style,dot_pixel);
      }
      break;
    case RIGHT_UPPER_CORNER:
    case LEFT_LOWER_CORNER:
      for (int y = Y_PIXELS - 1; y >= 0; y -= y_incr) {
         int px_end, py_end;
         DCT(&px_end,&py_end,X_PIXELS - 1,y,corner);
         draw_line(px_start,py_start,px_end,py_end,color,line_style,dot_pixel);	
      }      
    default: break;
  }
}

// draw set of lines varying X coordinate so as to 'sweep' along X axis...

void top_sweep(DISPLAY_CORNERS corner, int px_start, int py_start, int x_incr,
	       COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  switch(corner) {
    case LEFT_UPPER_CORNER:
    case RIGHT_LOWER_CORNER:
      for (int x = X_PIXELS - 1; x >= 0; x -= x_incr) {
         int px_end, py_end;
         DCT(&px_end,&py_end,x,Y_PIXELS - 1,corner);
         draw_line(px_start,py_start,px_end,py_end,color,line_style,dot_pixel);	
      }
      break;
    case RIGHT_UPPER_CORNER:
    case LEFT_LOWER_CORNER:
      for (int x = 0; x < X_PIXELS; x += x_incr) {
         int px_end, py_end;
         DCT(&px_end,&py_end,x,Y_PIXELS - 1,corner);
         draw_line(px_start,py_start,px_end,py_end,color,line_style,dot_pixel);	
      }
    default: break;
  }
}

void screen_sweep(DISPLAY_CORNERS corner,COLOR color,LINE_STYLE line_style,DOT_PIXEL dot_pixel) {
  int x_incr = 20;
  int y_incr = 20;

  // lines always start at logical coordinates 0,0...
  
  int px_start, py_start;
  DCT(&px_start,&py_start,0,0,corner);

  switch(corner) {
    case LEFT_UPPER_CORNER:
      side_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      top_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      break;
    case RIGHT_UPPER_CORNER:
      top_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      side_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      break;
    case RIGHT_LOWER_CORNER:
      side_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      top_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      break;
    case LEFT_LOWER_CORNER:
      top_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      side_sweep(corner,px_start,py_start,y_incr,color,line_style,dot_pixel);
      break;
    default: break;
  }

}
