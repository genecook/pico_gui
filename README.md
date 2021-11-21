line_sweeps.c - display sets of lines by color, sweeping around four
                corners of display.

display_bmp.c - display a single (bmp formatted) image on display.

  To create image from jpeg, used ImageMagick 'convert' utility as so:

    convert -verbose -resize 240x320! -rotate 180 mcook_designs.jpg md.bmp

  Resolution (240x320) specified for Waveshare 2.8 LCD.

track_touch.c - track pad example. Most of the vendor supplied track
  panel functions are private (declared static in LCD_Touch.c) and
  thus out of necessity copied and/or modified for this example.

chess.c, banner.c - proof of concept. Link in pico_chess c++ based chess
  engine, along with (c) code to display chessboard.
  
  
  