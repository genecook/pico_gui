void lcd_touch_startup();
void read_screen_touch(int *x, int *y);
void display_image(char *fname);
void wait(uint time_in_milliseconds);
void clear_screen();
enum { NO_FILL, DO_FILL }; // rectangle fill options
void draw_rectangle(uint16_t x, uint16_t y, uint16_t extent_x, uint16_t extent_y,
		    uint16_t color, uint16_t do_fill);
void draw_line(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end,uint16_t color,uint16_t line_style);

enum { COLOR_WHITE=0, COLOR_BLACK=1, COLOR_RED=2 }; 
enum { FONT_SIZE_12=12, FONT_SIZE_16=16 };
void display_string(uint16_t x, uint16_t y, const char *tbuf, uint16_t font_size,
		    uint16_t background_color, uint16_t foreground_color);
void display_char(uint16_t x, uint16_t y, const char tc, uint16_t font_size,
		  uint16_t background_color, uint16_t foreground_color);
#define FILE_RECORD_SIZE 128
int open_game_file(int for_write);
int close_game_file();
int write_to_game_file(const char *tbuf);
int read_from_game_file(char *tbuf);
int file_record_size();
#define NUM_PIXEL_ROWS  30
#define PIXELS_PER_ROW  30
int read_bmp30x30(uint16_t image_buffer[NUM_PIXEL_ROWS][PIXELS_PER_ROW], const char *bmp_file);
void draw_point(uint16_t x, uint16_t y, uint16_t image_point);
