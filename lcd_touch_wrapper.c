#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include "ff.h"
#include "fatfs_storage.h"

#include <string.h>
#include <lcd_touch_wrapper.h>

// locally defined touch support code:

void InitTouchPanel( LCD_SCAN_DIR Lcd_ScanDir );
void ReadTouch(POINT *x, POINT *y);

#ifdef USE_VERTICAL_DISPLAY
  LCD_SCAN_DIR lcd_scan_dir = L2R_U2D;
#else
  LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT;
#endif

LCD_SCAN_DIR bmp_scan_dir = D2U_R2L;      // LCD type

//***********************************************************************
// c wrapper functions for accessing lcd/touch...
//***********************************************************************

void lcd_touch_startup() {
  System_Init(); //System initialize, configure serial port and SPI interface...
  SD_Init();
  
  LCD_Init(lcd_scan_dir,800);   // Initialize LCD panel,
                                //   confirm the scan mode and the brightness
  InitTouchPanel(lcd_scan_dir); // Initialize touch panel

  GUI_Show();
}

// LCD agnostic access to touch...

void read_screen_touch(int *x, int *y) {
  POINT px = 260, py = 0;

  // x==260, y == 0 - appears to be reading when 'pen' leaves screen or goes
  // off edge. Keep trying 'til valid point set read...
  
  while( (px == 260) && (py == 0) ) {
    ReadTouch(&px, &py);
  }
  
  *x = px;
  *y = py;
}

// similar to API LCD_Show_bmp call, only this abbreviated version
// displays only a single image...

void display_image(char *fname) {
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

void wait(uint time_in_milliseconds) {
  Driver_Delay_ms(3000);
}

void clear_screen() {
  GUI_Clear(BLACK);
}

uint16_t true_color(uint16_t game_color) {
  switch(game_color) {
  case 0: return WHITE; break;
  case 1: return BLACK; break;
  case 2: return RED;   break;
  default: return game_color; break; // if not a recognized color index,
	                             // assume color is correct...
  }
  return WHITE;
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t extent_x, uint16_t extent_y,
		    uint16_t color, uint16_t do_fill) {
  GUI_DrawRectangle(x,y,extent_x,extent_y,true_color(color),
		    (do_fill == 1) ? DRAW_FULL : DRAW_EMPTY,
		    DOT_PIXEL_1X1);
}

void draw_line(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end,uint16_t color,uint16_t line_style) {
  GUI_DrawLine(x_start,y_start,x_end,y_end,color,LINE_SOLID,DOT_PIXEL_1X1);
}

// draw_point - color parameter MUST BE correct...

void draw_point(uint16_t x, uint16_t y, uint16_t color) {
  GUI_DrawPoint(x, y, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
}

sFONT* which_font(uint16_t font_size) {
  sFONT* TP_Font;
  switch(font_size) {
  case 12: TP_Font = &Font12; break;
  case 16: TP_Font = &Font16; break;
  default: break;
  }

  return TP_Font;
}

void display_string(uint16_t x, uint16_t y, const char *tbuf, uint16_t font_size,
		    uint16_t background_color, uint16_t foreground_color) {
  sFONT* TP_Font = which_font(font_size);

  GUI_DisString_EN(x,y,tbuf,TP_Font,
		   true_color(background_color), true_color(foreground_color));
}

void display_char(uint16_t x, uint16_t y, const char tc, uint16_t font_size,
		  uint16_t background_color, uint16_t foreground_color) {
  sFONT* TP_Font = which_font(font_size);

  GUI_DisChar(x,y,tc,TP_Font,
	      true_color(background_color), true_color(foreground_color));
}

// lifted from vendor LCD/flash-access code. Modified so as to display
// only a single BMP-formated image, read from flash...

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

// simple(minded) file i/o...

FIL game_file; 

int open_game_file(int for_write) {
  FRESULT fr = f_open(&game_file, "game.txt", (for_write ? (FA_WRITE | FA_CREATE_ALWAYS) : FA_READ) );
  if (fr != FR_OK)
    return -1;
  return 0;
}

int close_game_file() {
  if (f_close(&game_file) != FR_OK)
    return -1;
  return 0;
}

int write_to_game_file(const char *tbuf) {
  if (f_puts(tbuf, &game_file) == EOF)
    return -1;
  if (f_putc('\n',&game_file) == EOF) // newline is record separator
    return -1;
  return 0;
}

#define FILE_RECORD_SIZE 128

int file_record_size() { return FILE_RECORD_SIZE; };

int read_from_game_file(char *tbuf) {
  if (f_gets(tbuf, FILE_RECORD_SIZE, &game_file) != tbuf)
    return -1;
  if (strlen(tbuf) > 0)
    tbuf[strlen(tbuf) - 1] = '\0'; // strip newline record separator
  return 0;
}

//***********************************************************************
// ReadBMP30x30 - read graphic image, formatted as bmp
//                     of 30 x 30 pixels, 24 colors per pixel...
//***********************************************************************

// bmp file format as 30x30 pixels, 24 colors per pixel...

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

int read_bmp30x30(uint16_t image_buffer[NUM_PIXEL_ROWS][PIXELS_PER_ROW], const char *bmp_file) {
#ifdef DEBUG
  printf("[read_bmp30x30] entered...\n");
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
  printf("[read_bmp30x30] exited.\n");
#endif
  
  return 0; 
}



