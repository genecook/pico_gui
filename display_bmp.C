#include <stdio.h>
#include <iostream>
#include <string>
#include "pico/stdlib.h"

#include <chess.h>

//*****************************************************************************
// example pico lcd program - display 'BMP' image...
//*****************************************************************************

extern "C" {
  void GuiStartup();
  void DisplayImage(char *);
  void Wait(uint time_in_milliseconds);
  void ClearScreen();
}

int main(void) {
    GuiStartup();
    DisplayImage((char *) "chesbrd.bmp");
    Wait(3000);
    ClearScreen();
    stdio_init_all(); // don't need???
    
    std::cout << "Display BMP formatted image..." << std::endl;

    return 0;
}



    
