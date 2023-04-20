#include "main.h"   //Examples
#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include <stdio.h>
#include "hardware/watchdog.h"
#include "pico/stdlib.h"

//************************************************************************
// functions below are mostly copied/adapted from lib/lcd/LCD_Touch.c... 
//************************************************************************

extern LCD_DIS sLCD_DIS;
extern uint8_t id;
static TP_DEV  sTP_DEV;
static TP_DRAW sTP_Draw;

/*******************************************************************************
function:
                Use the default calibration factor
*******************************************************************************/

void my_TP_GetAdFac(void)
{
    if (LCD_2_8==id) {
        sTP_DEV.fXfac = 0.066626;
        sTP_DEV.fYfac = 0.089779 ;
        sTP_DEV.iXoff = -20 ;
        sTP_DEV.iYoff = -34 ;
    } else {
        if (sTP_DEV.TP_Scan_Dir == D2U_L2R ) { //SCAN_DIR_DFT = D2U_L2R
            sTP_DEV.fXfac = -0.132443 ;
            sTP_DEV.fYfac = 0.089997 ;
            sTP_DEV.iXoff = 516 ;
            sTP_DEV.iYoff = -22 ;         
        } else if( sTP_DEV.TP_Scan_Dir == L2R_U2D ) {
            sTP_DEV.fXfac = 0.089697 ;
            sTP_DEV.fYfac = 0.134792 ; 
            sTP_DEV.iXoff = -21 ;         
            sTP_DEV.iYoff = -39 ;
        } else if( sTP_DEV.TP_Scan_Dir == R2L_D2U ) {
            sTP_DEV.fXfac = 0.089915 ;
            sTP_DEV.fYfac =  0.133178 ;
            sTP_DEV.iXoff = -22 ;
            sTP_DEV.iYoff = -38 ;
        } else if( sTP_DEV.TP_Scan_Dir == U2D_R2L ) {
            sTP_DEV.fXfac = -0.132906 ;
            sTP_DEV.fYfac = 0.087964 ;
            sTP_DEV.iXoff = 517 ;
            sTP_DEV.iYoff = -20 ; 
        } else {
            LCD_Clear(LCD_BACKGROUND);
            GUI_DisString_EN(0, 60, "Does not support touch-screen \
                            calibration in this direction",
                            &Font16, FONT_BACKGROUND, RED);
        }   
    }
}
/*******************************************************************************
function:
                Touch pad initialization, get calibration factors for this LCD
*******************************************************************************/
void InitTouchPanel( LCD_SCAN_DIR Lcd_ScanDir ) {
  TP_Init(Lcd_ScanDir);
  my_TP_GetAdFac();
}
/*******************************************************************************
function:
                Read the ADC of the channel
parameter:
        Channel_Cmd :   0x90: Read channel Y +, select the ADC resolution is 
                              12 bits, set to differential mode
                        0xd0: Read channel x +, select the ADC resolution is 
                              12 bits, set to differential mode
*******************************************************************************/

uint16_t TP_Read_ADC(uint8_t CMD) {
    uint16_t Data = 0;

    //A cycle of at least 400ns.
    DEV_Digital_Write(TP_CS_PIN,0);

    SPI4W_Write_Byte(CMD);
    Driver_Delay_us(200);

    //  dont write 0xff, it will block xpt2046  
    //Data = SPI4W_Read_Byte(0Xff);
    Data = SPI4W_Read_Byte(0X00);
    Data <<= 8;//7bit
    Data |= SPI4W_Read_Byte(0X00);
    //Data = SPI4W_Read_Byte(0Xff);
    Data >>= 3;//5bit
    DEV_Digital_Write(TP_CS_PIN,1);
    return Data;
}
/*******************************************************************************
function:
                Read the 5th channel value and exclude the maximum and minimum returns the average
parameter:
        Channel_Cmd :   0x90 :Read channel Y +
                                        0xd0 :Read channel x +
*******************************************************************************/
#define READ_TIMES  5   //Number of readings
#define LOST_NUM    1   //Discard value
uint16_t TP_Read_ADC_Average(uint8_t Channel_Cmd)
{
    uint8_t i, j;
    uint16_t Read_Buff[READ_TIMES];
    uint16_t Read_Sum = 0, Read_Temp = 0;
    //LCD SPI speed = 3 MHz
    spi_set_baudrate(SPI_PORT,3000000);
    //Read and save multiple samples
    for(i = 0; i < READ_TIMES; i++){
                Read_Buff[i] = TP_Read_ADC(Channel_Cmd);
                Driver_Delay_us(200);
        }
    //LCD SPI speed = 18 MHz
    spi_set_baudrate(SPI_PORT,18000000);
    //Sort from small to large
    for (i = 0; i < READ_TIMES  -  1; i ++) {
        for (j = i + 1; j < READ_TIMES; j ++) {
            if (Read_Buff[i] > Read_Buff[j]) {
                Read_Temp = Read_Buff[i];
                Read_Buff[i] = Read_Buff[j];
                Read_Buff[j] = Read_Temp;
            }
        }
    }

    //Exclude the largest and the smallest
    for (i = LOST_NUM; i < READ_TIMES - LOST_NUM; i ++)
        Read_Sum += Read_Buff[i];

    //Averaging
    Read_Temp = Read_Sum / (READ_TIMES - 2 * LOST_NUM);

    return Read_Temp;
}
/*******************************************************************************
function:
                Read X channel and Y channel AD value
parameter:
        Channel_Cmd :   0x90 :Read channel Y +
                                        0xd0 :Read channel x +
*******************************************************************************/
void TP_Read_ADC_XY(uint16_t *pXCh_Adc, uint16_t  *pYCh_Adc )
{
    *pXCh_Adc = TP_Read_ADC_Average(0xD0);
    *pYCh_Adc = TP_Read_ADC_Average(0x90);
}
/*******************************************************************************
function:
                2 times to read the touch screen IC, and the two can not exceed the deviation,
                ERR_RANGE, meet the conditions, then that the correct reading, otherwise the reading error.
parameter:
        Channel_Cmd :   pYCh_Adc = 0x90 :Read channel Y +
                                        pXCh_Adc = 0xd0 :Read channel x +
*******************************************************************************/
#define ERR_RANGE 50    //tolerance scope
bool TP_Read_TwiceADC(uint16_t *pXCh_Adc, uint16_t  *pYCh_Adc )
{
    uint16_t XCh_Adc1, YCh_Adc1, XCh_Adc2, YCh_Adc2;

    //Read the ADC values Read the ADC values twice
    TP_Read_ADC_XY(&XCh_Adc1, &YCh_Adc1);
        Driver_Delay_us(10);
    TP_Read_ADC_XY(&XCh_Adc2, &YCh_Adc2);
        Driver_Delay_us(10);

    //The ADC error used twice is greater than ERR_RANGE to take the average
    if( ((XCh_Adc2 <= XCh_Adc1 && XCh_Adc1 < XCh_Adc2 + ERR_RANGE) ||
         (XCh_Adc1 <= XCh_Adc2 && XCh_Adc2 < XCh_Adc1 + ERR_RANGE))
        && ((YCh_Adc2 <= YCh_Adc1 && YCh_Adc1 < YCh_Adc2 + ERR_RANGE) ||
            (YCh_Adc1 <= YCh_Adc2 && YCh_Adc2 < YCh_Adc1 + ERR_RANGE))) {
        *pXCh_Adc = (XCh_Adc1 + XCh_Adc2) / 2;
        *pYCh_Adc = (YCh_Adc1 + YCh_Adc2) / 2;
        return true;
    }

    //The ADC error used twice is less than ERR_RANGE returns failed
    return false;
}
//****************************************************************************
// read from touch panel, return X,Y coordinates...
//****************************************************************************

bool ReadTouchEvent(POINT *x, POINT *y, bool return_raw_coordinates);

void ReadTouch(POINT *x, POINT *y) {
    ReadTouchEvent(x,y,false); // but probably not for the 3.5" lcd with touch)
}

// return true if good read of touch...

bool ReadTouchEvent(POINT *x, POINT *y, bool return_raw_coordinates) {
    if (DEV_Digital_Read(TP_IRQ_PIN)) {
        return false; // problem with read?
    }

    // read the current stylus position, raw touch panel coordinates...
    if (!TP_Read_TwiceADC(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint)) {
        return false; // read errors?
    }

    if (return_raw_coordinates) {
        // return the touch panel 'raw' coordinate values only...
        *x = sTP_DEV.Xpoint;
        *y = sTP_DEV.Ypoint;
    } else if (id == LCD_2_8) {
        // return screen coordinates - LCD 2.8 only...
        *x = sLCD_DIS.LCD_Dis_Column - sTP_DEV.fXfac * sTP_DEV.Xpoint - sTP_DEV.iXoff;
        *y = sLCD_DIS.LCD_Dis_Page   - sTP_DEV.fYfac * sTP_DEV.Ypoint - sTP_DEV.iYoff;
    } else if (sTP_DEV.TP_Scan_Dir == R2L_D2U) {
        // screen coordinates, 'scanning' right to left, down to up...
        *x = sTP_DEV.fXfac * sTP_DEV.Xpoint + sTP_DEV.iXoff;
        *y = sTP_DEV.fYfac * sTP_DEV.Ypoint + sTP_DEV.iYoff;
    } else if (sTP_DEV.TP_Scan_Dir == U2D_R2L) {
        // screen coordinates, 'scanning' up to down, right to left...
        *x = sTP_DEV.fXfac * sTP_DEV.Ypoint + sTP_DEV.iXoff;
        *y = sTP_DEV.fYfac * sTP_DEV.Xpoint + sTP_DEV.iYoff;
    } else if (sTP_DEV.TP_Scan_Dir == L2R_U2D) {
        // screen coordinates, 'scanning' left to right, up to down...
        *x = sLCD_DIS.LCD_Dis_Column - sTP_DEV.fXfac * sTP_DEV.Xpoint - sTP_DEV.iXoff;
        *y = sLCD_DIS.LCD_Dis_Page   - sTP_DEV.fYfac * sTP_DEV.Ypoint - sTP_DEV.iYoff;
    } else {
        // screen coordinates, 'scanning' down to up, left to right...
        *x = sLCD_DIS.LCD_Dis_Column - sTP_DEV.fXfac * sTP_DEV.Ypoint - sTP_DEV.iXoff;
        *y = sLCD_DIS.LCD_Dis_Page   - sTP_DEV.fYfac * sTP_DEV.Xpoint - sTP_DEV.iYoff;
    }

    return true;
}

