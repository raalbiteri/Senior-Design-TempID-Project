/**************************************************************************
 * TempID ST7735r Graphical LCD API
 * Created: 1/03/21
 * Author: Brycen Hakes
 * Provides: An LCD API usable by the TempID driver, includes functions
 *           used to interact with the LCD
 **************************************************************************/

#include "LCD.h"

// Pin definitions for SPI wiring
// For the breakout board, you can use any 2 or 3 pins.
#define TFT_CS        10
#define TFT_RST        9
#define TFT_DC         6

// using the HARDWARE SPI pins, which are unique.
// This is the fastest mode of operation and is required if
// using the breakout board's microSD card.
// Another option is to use GPIO pins which will slow operation,
// but allow for other components to use SPI pins

//Creating tft object with pins
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//Temperature Units - F by default
char units = 'F';


/*
 * Function to initialize LCD and set up for
 * reading and printing User temperatures
 */
void LCD::initLCD() {
  //Serial.begin(9600); //open the serial port at 9600 bps

  // Initialize LCD
  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab

  //Serial.println(F("Initialized"));

  // Intro Screen
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setCursor(10, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("TempID");
  tft.setCursor(18, 30);
  delay(1000);

}


/*
 * Prints a temperature sample to the LCD.
 *
 * @Param: sample - a double  which will be printed
 *         to the LCD representing a temperature.
 *
 * @Returns: void
 */
void LCD::printSample(double sample){
  if(sample >= 100){
    //Convert int to String
    String strSample = (String) sample;
    //Large text size for visibility
    tft.setTextSize(3);
    //Clear screen (fill black)
    tft.fillRect(0,30,130,105, ST77XX_BLACK);
    //Cursor moves to designated printing area
    tft.setCursor(10,60);
    //Print sample
    tft.print(strSample);
  } else {
    //Convert int to String
    String strSample = (String) sample;
    //Large text size for visibility
    tft.setTextSize(3);
    //Clear screen (fill black)
    tft.fillRect(0,30,130,105, ST77XX_BLACK);
    //Cursor moves to designated printing area
    tft.setCursor(20,60);
    //Print sample
    tft.print(strSample);
    printUnits();
  }

}


/*
* Prints a battery graphic and percentage value on
* the LCD screen in the upper right hand corner
*
* @Param: uint8_t percentage - the battery level in
*         a percentage format
*
* @Returns: none
*/
void LCD::printBattery(uint8_t percentage){
  //Clear battery spot
  tft.fillRect(33,8,40,20,ST77XX_BLACK);
  double pixelConversion = 5.26;
  //Error check battery level
  if(percentage > 100){
    percentage = 100;
  }
    //Using int because fractions of pixels does not exist
    int16_t percentConverted = percentage/pixelConversion;
    //Draw shape (xpixel, ypixel, width, height, color);
    tft.drawRect(8,10,20,12,ST77XX_GREY);
    //Add battery tip
    tft.drawRect(28,13,2,6,ST77XX_GREY);
    //Fill battery in
    tft.fillRect(9,11,percentConverted,10,ST77XX_WHITE);

    //Write percentage
    //Set small font size
    tft.setTextSize(1);
    //Move cursor after battery
    tft.setCursor(34,12);
    //print percentage as string
    tft.print((String)percentage);
    //print percent symbol
    tft.write('%');



}

/*
 * Used to swap units between F/C
 *
 * @Param: none
 *
 * @Return void
 */
 void LCD::swapUnits(){
  if(units == 'F'){
    units = 'C';
  } else {
    units = 'F';
  }
 }


 /*
 * Prints units to the top right corner of the LCD
 *
 * @Params: none
 *
 * @Returns: none
 */
 void LCD::printUnits(){
   //Move to upper right corner
   tft.setCursor(98, 6);
   //Set small font size
   tft.setTextSize(2);
   //Print degrees symbol
   tft.print("\370");
   //Print units
   tft.setCursor(108, 10);
   tft.print(units);
 }

/*
 * Clears the LCD screen to black
 *
 * @Param: none
 *
 * @Return: void
 *
 */
 void LCD::clearScreen(){
  tft.fillScreen(ST77XX_BLACK);
 }

 /*
 * Prints the intro animation for TempID
 *
 * @Param: none
 *
 * @Return: none
 */
 void LCD::animation(){
   tft.drawCircle(113,40,10,ST77XX_WHITE);//1
   delay(80);
   tft.drawLine(80,35,10,35,ST77XX_WHITE);//2
   delay(80);
   tft.drawLine(10,35,10,50,ST77XX_WHITE);//3
   delay(80);
   tft.drawLine(10,50,30,50,ST77XX_WHITE);//4
   delay(80);
   tft.drawLine(30,50,30,100,ST77XX_WHITE);//5
   delay(80);
   tft.drawLine(30,100,10,100,ST77XX_WHITE);//6
   delay(80);
   tft.drawLine(10,100,10,115,ST77XX_WHITE);//7
   delay(80);
   tft.drawLine(10,115,80,115,ST77XX_WHITE);//8
   delay(80);
   tft.drawLine(80,115,100,100,ST77XX_WHITE);//9
   delay(80);
   tft.drawLine(100,100,100,50,ST77XX_WHITE);//10
   delay(80);
   tft.drawLine(100,50,80,35,ST77XX_WHITE);//11
   delay(80);
   tft.drawLine(70,35,70,50,ST77XX_WHITE);//12
   delay(80);
   tft.drawLine(70,50,50,50,ST77XX_WHITE);//13
   delay(80);
   tft.drawLine(50,50,50,100,ST77XX_WHITE);//14
   delay(80);
   tft.drawLine(50,100,70,100,ST77XX_WHITE);//15
   delay(80);
   tft.drawLine(70,100,70,115,ST77XX_WHITE);//16
   delay(80);
   delay(4000);
   clearScreen();
 }

 /*
 * Draws bluetooth status symbol
 *
 * @Param: none
 *
 * @Return: void
 *
 */
 void LCD::btSymbol(){

   tft.drawTriangle(86,8,92,13,86,17,ST77XX_WHITE);
   tft.drawTriangle(86,24,92,19,86,15,ST77XX_WHITE);
   tft.drawLine(86,15,80,11,ST77XX_WHITE);
   tft.drawLine(86,17,80,21,ST77XX_WHITE);

 }

 /*
 * Removes bluetooth status symbol
 *
 * @Param: none
 *
 * @Return: void
 *
 */
 void LCD::btSymbolClear(){
   tft.fillRect(85,7,12,12,ST77XX_BLACK);
 }

 /*
 * Display a charging symbol so the user knows their device
 * is currently being charged
 *
 * @Param: none
 *
 * @Returns: void
 */
 void LCD::charging(){
   //Clear battery spot
   tft.fillRect(33,8,40,20,ST77XX_BLACK);
   //Draw shape (xpixel, ypixel, width, height, color);
   tft.drawRect(8,10,20,12,ST77XX_GREY);
   //Add battery tip
   tft.drawRect(28,13,2,6,ST77XX_GREY);
   //Fill battery in
   tft.fillRect(9,11,18,10,ST77XX_GREEN);
   //Draw symbol over battery

 }
