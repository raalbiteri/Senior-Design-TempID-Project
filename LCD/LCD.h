/**************************************************************************
 * TempID ST7735r Graphical LCD API Header
 * Created: 1/03/21
 * Author: Brycen Hakes
 * Provides: An LCD API usable by the TempID driver, includes functions
 *           used to interact with the LCD
 **************************************************************************/

#ifndef LCD_H
#define LCD_H

 //Inclusions
 #include <Adafruit_GFX.h>    // Core graphics library
 #include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
 #include <SPI.h>             // SPI library for communication

class LCD{

public:


/*
* Function to initialize LCD and set up for
* reading and printing User temperatures
*
* @Param: none
*
* @Returns: void
*/
void initLCD();


/*
* Prints a temperature sample to the LCD.
*
* @Param: sample - a double which will be printed
*         to the LCD representing a temperature.
*
* @Returns: void
*/
void printSample(double sample);

/*
* Prints a battery graphic and percentage value on
* the LCD screen in the upper right hand corner
*
* @Param: uint8_t percentage - the battery level in
*         a percentage format
*
* @Returns: none
*/
void printBattery(uint8_t percentage);

/*
* Prints units to the top right corner of the LCD
*
* @Params: none
*
* @Returns: none
*/
void printUnits();

/*
* Used to swap units between F/C
*
* @Param: none
*
* @Return void
*/
void swapUnits();


/*
 * Clears the LCD screen to black
 *
 * @Param: none
 *
 * @Return: void
 *
 */
 void clearScreen();

 /*
 * Prints the intro animation for TempID
 *
 * @Param: none
 *
 * @Return: none
 */
 void animation();

 /*
 * Draws bluetooth status symbol
 *
 * @Param: none
 *
 * @Return: void
 *
 */
 void btSymbol();

 /*
 * Removes bluetooth status symbol
 *
 * @Param: none
 *
 * @Return: void
 *
 */
 void btSymbolClear();

/*
* Display a charging symbol so the user knows their device
* is currently being charged
*
* @Param: none
*
* @Returns: void
*/
void charging();

};



 #endif //LCD_H
