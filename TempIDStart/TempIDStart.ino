#include <Adafruit_LittleFS.h>
#include <Adafruit_NeoPixel.h>
#include <arduino-timer.h>
#include <bluefruit.h>
#include <EasyButton.h>
#include <InternalFileSystem.h>
#include <LCD.h>
#include <TemperatureSensors.h>

/**************************************************************************
 * TempIDStart Main Arduino Code
 * Created: 1/03/21
 * Last Update: 04/20/21
 * Author: Raunel Albiter, Brycen Hakes, Jacob Schultz
 * Provides: TempID driver for Bluetooth Connectivity, LCD display,
 *           temperature readings, and Neopixel status change
 **************************************************************************/


/**
 * The definitions below is meant for getting battery voltages
 * and some conversion constants to get battery as percentage
 * Credit: Adafruit nrf52840 ADC https://learn.adafruit.com/adafruit-feather-sense/nrf52-adc
 */
#if defined ARDUINO_NRF52840_CIRCUITPLAY
#define  PIN_VBAT          A6                                                     // this is just a mock read, we'll use the light sensor, so we can run the test
#endif

#define VBAT_MV_PER_LSB   (0.73242188F)                                           // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096

#ifdef NRF52840_XXAA
#define VBAT_DIVIDER      (0.5F)                                                  // 150K + 150K voltage divider on VBAT
#define VBAT_DIVIDER_COMP (2.0F)                                                  // Compensation factor for the VBAT divider
#else
#define VBAT_DIVIDER      (0.71275837F)                                           // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)                                                // Compensation factor for the VBAT divider
#endif

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
uint32_t vbat_pin = PIN_VBAT;                                                     //TODO: A7 for feather nRF52832, A6 for nRF52840

/**
 * The definition below are meant to make delays clearer.
 *  Factors are identified for desired delay time and if
 *  different delay is needed the admin must only need to
 *  change these definition values
 *  Note: Leave either value at '1' to fine tune particular
 *  delay factor
 *    Ex: 5 hours =
 *          MILLISECOND_FACTOR = 1000
 *          SECONDS_FACTOR = 60
 *          MINUTES_FACTOR = 60
 *          HOUR_FACTOR = 5
 *    Ex: 5 minutes =
 *          MILLISECOND_FACTOR = 1000
 *          SECONDS_FACTOR = 60
 *          MINUTES_FACTOR = 5
 *          HOUR_FACTOR = 1
 *    Ex: 5 seconds =
 *          MILLISECOND_FACTOR = 1000
 *          SECONDS_FACTOR = 5
 *          MINUTES_FACTOR = 1
 *          HOUR_FACTOR = 1
 *    Ex: 5 milliseconds =
 *          MILLISECOND_FACTOR = 5
 *          SECONDS_FACTOR = 1
 *          MINUTES_FACTOR = 1
 *          HOUR_FACTOR = 1
 */
#define MILLISECOND_FACTOR 1000
#define SECONDS_FACTOR 30
#define MINUTES_FACTOR 1
#define HOUR_FACTOR 1

/**
 * The definitions below are meant for the NeoPixel
 *  status color changes and setup. Sensor data is
 *  subtracted by a factor of 15*F (8*C) for testing
 *  purposes only until enclosure is built to actually
 *  account for body temps 
 * (TODO: Delete subtracting factor)
 */
#define LED_COUNT 1
#define MAX_BRIGHT 250
#define DEFAULT_BRIGHTNESS 5
#define FEVER_F (100.6F-15.0F)
#define NORMAL_MIN_F (97-15)
#define NORMAL_MAX_F (99-15) 


/**
 * Global Variable Values/Objects:
 *  Provide necessary data for setup of delays,
 *    LCD, Bluetooth, temperature sensors, and Neopixel
 */

// Bluetooth Variables
BLEDfu  bledfu;                                                                   // OTA DFU service
BLEDis  bledis;                                                                   // device information
BLEUart bleuart;                                                                  // uart over ble
BLEBas  blebas;                                                                   // battery
bool readInfrared = false;                                                        // Whether infrared temp needs to be sent
int sensor_delay = MILLISECOND_FACTOR*SECONDS_FACTOR*MINUTES_FACTOR*HOUR_FACTOR;  // Desired time delay between readings 
char notificationBuffer [10];                                                     // BLE transmit buffer
bool bluetoothConnected = false;                                                  // Whether device is connected via Bluetooth

// LCD Variables
LCD lcd;                                                                          // LCD Object

// Sensor Variables
TemperatureSensors ts;                                                            // Temperature sensor object
bool tempSensorSelect = false;                                                            // true for MLX, false for DS
double irTemp = 0.0;

// Neopixel Variables
Adafruit_NeoPixel neo(LED_COUNT, PIN_NEOPIXEL, NEO_RGB + NEO_KHZ800);             // Neopixel color format is 24 bit GRB (Green | Red | Blue)
uint32_t green = neo.Color(255,0,0);                                              // Color Palette
uint32_t red = neo.Color(0,255,0);
uint32_t blue = neo.Color(0,0,255);
uint32_t yellow = neo.Color(255,255,0);
uint32_t other = neo.Color(0,255,255);
uint32_t black = neo.Color(0,0,0);                                                
const int FLASH_INTERVAL = 1400;                                                  // ON/OFF timing for warning flash
bool neoToggle = false;                                                           // ON/OFF toggle switch
char currentColor = 'b';                                                          // Current/Previous color of NeoPixel
char previousColor = 'b';

// Button Variables
const int POWERBUTTON = 5;                                                        // Pin number of external button
EasyButton button(POWERBUTTON);                                                   // New instance of class for interactive button simplicity
const int LONG_PRESS = 3000;                                                      // Length of time for toggling power (3 second button hold)
bool powerON = false;                                                             // Whether TempID is ON/OFF

// Timer variables
auto timer = timer_create_default();                                              // Timer object for running key tasks (LCD change, Flash LED, Take temperatures)
bool tempStatusChange(void *);                                                    // Method for updating color of Neopixel
bool warningFlash(void *);
auto changeColor = timer.every(FLASH_INTERVAL, tempStatusChange);            // Task for flashing LED for warning
auto flashLED = timer.every(FLASH_INTERVAL + 1000, warningFlash);
bool initialStart = true;                                                         // Whether the TempID is running for the first time
bool warningCancelled = false;

/**
 * Initial setup of all devices include BLE, LCD,
 *  temperature sensors, and Neopixel
 * BLE Code Credit: Nordic Semiconductor
 */
void setup() {
  Serial.begin(115200);
  
  while(!Serial); //debugging only
  introMessage(); //debugging only  
  
  // Configure BLE
  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);
 
  // Config the peripheral connection with maximum bandwidth 
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
 
  Bluefruit.begin();
  Bluefruit.setTxPower(-4);    // Check bluefruit.h for supported values

  Bluefruit.setName("TempID#001");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
 
  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();
 
  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();
 
  // Configure and Start BLE Uart Service
  bleuart.begin();
 
  // Start BLE Battery Service (100% USB power)
  // Get a single ADC sample and throw it away
  blebas.begin();
  readVBAT();

  // Set up LCD
  lcd.initLCD();
  lcd.clearScreen();  

  // Set up Temperature Sensors
  ts.initSensors();

  //Set up Neopixel at desired brightness (Default/Max)
  neo.setBrightness(DEFAULT_BRIGHTNESS);
  neo.begin();

  // Set up button (Long/Short press functions)
  button.begin();                                                                    
  button.onPressedFor(LONG_PRESS, powerToggle);
  button.onPressed(readIRTemp);
}

/**
 * Temperature readings are being logged in the background but
 * once the TempID application decides to subscribe to notifications
 * temperature data will be send using the appropriate ble function (bleuart)
 * Temperature/Battery monitoring interval can be changed by adjusting sensor_delay (minutes)
 * Temperature data prefixed with 'P ' and battery data prefixed with 'B '
 */
void loop() {

  // Don't turn on until long press is detected
  while(!powerON){
    button.read();
    delay(200);
  }

  // On initial start update the temperature/battery
  if(initialStart) {
    updateTemperature(NULL);
    delay(200);
    updateBattery(NULL);
    initialStart = false;
  }

  // Make TempID discoverable, tick the timer, and listen for button presses
  startAdv();
  timer.tick();
  button.read();

  // Send infrared temp to app if read through interrupt
  if(readInfrared and bluetoothConnected) {
    Serial.println("Sending Infrared Temp..."); //debugging only
    notificationBuffer[0] = '\0'; //Clear BLE buffer
    uint8_t tempInf = (uint8_t) irTemp;
    tempSensorSelect = true;
    snprintf(notificationBuffer, sizeof(notificationBuffer) - 1, "T %d", tempInf);
    bleuart.write(notificationBuffer);
    readInfrared = false; //Done sending infrared set as false
  }
}

/**
 * Function that acts as a software power toggle for
 *  stopping advertising and stopping display on LCD
 */
void powerToggle() {
  if(!powerON) {
    Serial.println("POWERING ON!"); //debugging only
    lcd.animation();
    timer.every(sensor_delay, updateTemperature);
    timer.every(sensor_delay + 100, updateBattery);
    timer.every(sensor_delay + 160, updateBleSymbol);
    if(!initialStart) {
      changeColor = timer.every(FLASH_INTERVAL, tempStatusChange);
      flashLED = timer.every(FLASH_INTERVAL + 1000, warningFlash);
    }
    neo.setPixelColor(0, blue);
    neo.show();
    currentColor = 'l';
    powerON = true;
  } else {
    Serial.println("POWERING OFF!"); //debugging only
    Bluefruit.Advertising.stop();
    timer.cancel();
    lcd.clearScreen();
    neo.setPixelColor(0, black);
    neo.show();
    currentColor = 'b';
    powerON = false;
    initialStart = false;
  }
}

/**
 * Helper function used to begin advertising the Featherboard
 *  device (make discoverable) by other BLE devices
 * Credit: Nordic Semiconductor
 */
void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
 
  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);
 
  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

/**
 * Callback function that updates the status of connection
 *  to the user and lets them know which device they are
 *  connected to
 * @param conn_handle connection where this event happens
 * Credit: Nordic Semiconductor
 */
void connect_callback(uint16_t conn_handle) {
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
 
  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
 
  Serial.print("Connected to ");
  Serial.println(central_name);
  bluetoothConnected = true;
  lcd.btSymbol();
}
 
/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 * Credit: Nordic Semiconductor
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void) conn_handle;
  (void) reason;
  notificationBuffer[0] = '\0';
  Serial.println();
  Serial.print("Disconnected, reason = 0x"); 
  Serial.println(reason, HEX);
  bluetoothConnected = false;
  //lcd.btSymbolClear();
}

/**
 * One of the main functions for updating the display to show
 *  a Bluetooth logo when TempID is connected/disconnected from
 *  the Android app
 */
bool updateBleSymbol(void *) {

  if(bluetoothConnected) {
    lcd.btSymbol();
  } else {
    lcd.btSymbolClear();
  }
  return true;
}

/**
 * Button triggered interrupt function that reads
 *  temperature in from infrared sensor and prints
 *  to LCD, later will send to app in main loop.
 */
void readIRTemp() {

  if(!powerON) {
    return;
  }
  readInfrared = true;
  tempSensorSelect = true;
  irTemp = (double)ts.readInfraredTemp();
  lcd.clearScreen();
  lcd.printSample(irTemp);
  lcd.printUnits();
}

/**
 * Helper function that will flash the yellow or
 *  specified 'other' color when a warning needs to
 *  be sent out to user
 */
bool warningFlash(void *) {

  neoToggle = not neoToggle;
  
  if((currentColor == 'y' or currentColor == 'o') and neoToggle) {
    neo.setPixelColor(0, black);
    neo.show();
    currentColor = 'b';
  }
  return true;
}

/**
 * Helper function that will change the color of
 *  the neopixel based on the extremities. Colors
 *  include GREEN(Normal), YELLOW(Caution), RED(FEVER) 
 * @param temp read temperature (ambient or infrared)
 */
bool tempStatusChange(void *) {

  float temp = 0.0;
  if(tempSensorSelect) {
    temp = irTemp;
    tempSensorSelect = false;
  } else {
    temp = (float)ts.contactSensorLoop();
  }

  if(temp >= FEVER_F) {
    neo.setPixelColor(0, red);
    currentColor = 'r';
    warningCancelled = true; 
  } else if(temp > NORMAL_MAX_F) {
    neo.setPixelColor(0, yellow);
    currentColor = 'y';
    warningCancelled = false;
  } else if(temp < NORMAL_MIN_F) {
    neo.setPixelColor(0, other);
    currentColor = 'o';
    warningCancelled = false;
  } else {
    neo.setPixelColor(0, green);
    currentColor = 'g';
    warningCancelled = true;
  }
  
  if((currentColor == 'y' or currentColor == 'o') and 
       not warningCancelled and not (previousColor == currentColor)) {
    timer.cancel(flashLED);
    flashLED = timer.every(FLASH_INTERVAL, warningFlash);
  } else if(not (currentColor == 'y' or currentColor == 'o') and not warningCancelled) {
    timer.cancel(flashLED);
    warningCancelled = true;
  }

  previousColor = currentColor;
  neo.show();
  return true;
}

/**
 * One of the main functions for getting updated temperature
 *  from DSB18B20 and both printing on LCD and sending data
 *  via Bluetooth to Android Application. Function must be
 *  used with arduino-timer functions for TempID purposes
 */
bool updateTemperature(void *) {

  //Send temperature to LCD for display
  double ds_tempF = ts.contactSensorLoop();
  tempSensorSelect = false;
  
  lcd.clearScreen();
  lcd.printSample(ds_tempF);
  lcd.printUnits();

  //Send/Print temperature (will only write to ble if user suscribed)
  notificationBuffer[0] = '\0'; //Clear BLE buffer
  uint8_t temp = (uint8_t) ds_tempF; //Convert double to uint8_t
  snprintf(notificationBuffer, sizeof(notificationBuffer) - 1, "T %d", temp);
  bleuart.write(notificationBuffer);
  return true;
}


/**
 * Function used to read out the battery voltage on pins
 * A6, A7, or A8 (for nrf52840 it should be A6)
 * Credit: Adafruit nrf52840 ADC https://learn.adafruit.com/adafruit-feather-sense/nrf52-adc
 */
float readVBAT(void) {
  float raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vbat_pin);

  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  return raw * REAL_VBAT_MV_PER_LSB;
}

/**
 * Function used to convert the battery voltage into
 * a percentage value that will be sent over to the 
 * application for cleaner display
 * @param mvolts Voltage of battery analog readout in (mV)
 * Credit: Adafruit nrf52840 ADC https://learn.adafruit.com/adafruit-feather-sense/nrf52-adc
 */
uint8_t mvToPercent(float mvolts) {
  if(mvolts<3300)
    return 0;

  if(mvolts <3600) {
    mvolts -= 3300;
    return mvolts/30;
  }

  mvolts -= 3600;
  return 10 + (mvolts * 0.15F );  // thats mvolts /6.66666666
}

/**
 * One of the main functions for getting updated battery percentage
 *  from analog sensor and both printing on LCD and sending data
 *  via Bluetooth to Android Application. Function must be
 *  used with arduino-timer functions for TempID purposes
 */
bool updateBattery(void *) {
  // Get a raw ADC reading
  float vbat_mv = readVBAT();
  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_mv);

  //Send battery percentage to LCD for display
  lcd.printBattery(vbat_per);
  
  //Send/Print battery percentage (will only write to ble if user subscribed)
  notificationBuffer[0] = '\0'; //Clear BLE buffer
  snprintf(notificationBuffer, sizeof(notificationBuffer) - 1, "B %d", vbat_per);
  bleuart.write(notificationBuffer);
  Serial.println("Battery Percentage: "); //debugging only
  Serial.print(vbat_per); //debugging only
  Serial.println("%"); //debugging only
  return true;
}

/**
 * Debugging function meant to print out initial message
 *  to console in order to check that Featherboard is ready
 */
void introMessage() {
  Serial.println("Temperature Identification (TempID)");
  Serial.println("-------------------------------------\n");
  Serial.println("Please use TempID Starter App to connect");
}
