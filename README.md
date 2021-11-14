line_sweeps.c - display sets of lines by color, sweeping around four
                corners of display.

display_bmp.c - display a single (bmp formatted) image on display.

  To create image from jpeg, used ImageMagick 'convert' utility as so:

    convert -verbose -resize 240x320! mcook_designs.jpg md.bmp

  Resolution (240x320) specified for Waveshare 2.8 LCD.
  
  