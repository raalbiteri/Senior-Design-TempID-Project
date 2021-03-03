#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Adafruit_MLX90614.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <Wire.h>
#include "LCD.h"

/**************************************************************************
 * TempIDStart Main Arduino Code
 * Created: 1/03/21
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
#define SECONDS_FACTOR 10
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
#define FEVER_C (38-8)
#define NORMAL_MIN_C (36-8)
#define NORMAL_MAX_C (37-8) 


/**
 * Global Variable Values/Objects:
 *  Provide necessary data for setup of delays,
 *    LCD, Bluetooth, temperature sensors, and Neopixel
 */

BLEDfu  bledfu;                                                                   // OTA DFU service
BLEDis  bledis;                                                                   // device information
BLEUart bleuart;                                                                  // uart over ble
BLEBas  blebas;                                                                   // battery
bool readInfrared = false;                                                        // Whether infrared temp needs to be sent
bool isFarenheit = true;                                                          // Current Units
int sensor_delay = MILLISECOND_FACTOR*SECONDS_FACTOR*MINUTES_FACTOR*HOUR_FACTOR;  // Desired time delay between readings 
char notificationBuffer [10];                                                     // BLE transmit buffer

LCD lcd;                                                                          // LCD Object

const int USERBUTTON = 7;                                                         // pin number of pushbutton on board
const int DS18B20_SENSOR_PIN = 9;                                                 // pin on Arduino connected to DS18B20's sensor pin
Adafruit_MLX90614 MLX90614 = Adafruit_MLX90614();                                 // new instance of class
OneWire oneWire(DS18B20_SENSOR_PIN);                                              // new instance of oneWire class
DallasTemperature DS18B20(&oneWire);                                              // pass oneWire reference to DallasTemperature library
DeviceAddress DS18B20_Addr;                                                       // address of DS18B20
float ds_tempC;                                                                   // DS18B20 temperature in Celsius
float ds_tempF;                                                                   // DS18B20 temperature in Fahrenheit
float mlx_tempC;                                                                  // MLX90614 temperature in Celsius
float mlx_tempF;                                                                  // MLX90614 temperature in Fahrenheit

Adafruit_NeoPixel neo(LED_COUNT, PIN_NEOPIXEL, NEO_RGB + NEO_KHZ800);             // Neopixel color format is 24 bit GRB (Green | Red | Blue)
uint32_t green = neo.Color(255,0,0);                                              
uint32_t red = neo.Color(0,255,0);
uint32_t blue = neo.Color(0,0,255);
uint32_t yellow = neo.Color(255,255,0);
uint32_t black = neo.Color(0,0,0);


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
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

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
  DS18B20.begin();                                                                // initialize contact sensor
  MLX90614.begin();                                                               // initialize infrared sensor
  oneWire.reset_search();                                                         // reset oneWire search
  if(!oneWire.search(DS18B20_Addr)){                                              // find address of DS18B20
    Serial.print("Unable to find Address for DS18B20 Sensor"); 
  }
  pinMode(USERBUTTON, INPUT_PULLUP);                                              // set user button to input
  attachInterrupt(digitalPinToInterrupt(USERBUTTON), readInfraredTemp, RISING);   // interrupt for button press

  //Set up Neopixel at desired brightness (Default/Max)
  neo.setBrightness(DEFAULT_BRIGHTNESS);
  neo.begin();
  
  // Set up and start advertising BLE
  startAdv();
}

/**
 * Temperature readings are being logged in the background but
 * once the TempID application decides to subscribe to notifications
 * temperature data will be send using the appropriate ble function (bleuart)
 * Temperature/Battery monitoring interval can be changed by adjusting sensor_delay (minutes)
 * Temperature data prefixed with 'P ' and battery data prefixed with 'B '
 */
void loop() {

  //Send infrared temp to app if read through interrupt
  if(readInfrared) {
    Serial.println("Sending Infrared Temp..."); //debugging only
    notificationBuffer[0] = '\0'; //Clear BLE buffer
    uint8_t tempInf = (uint8_t) mlx_tempF;
    tempStatusChange(mlx_tempF);
    snprintf(notificationBuffer, sizeof(notificationBuffer) - 1, "T %d", tempInf);
    bleuart.write(notificationBuffer);
    readInfrared = false; //Done sending infrared set as false
    delay(sensor_delay); //15 seconds
  }

  //Update temperature here
  DS18B20.requestTemperatures();              
  ds_tempC = DS18B20.getTempC(DS18B20_Addr);                                      
  ds_tempF = DS18B20.toFahrenheit(ds_tempC);

  //Send temperature to LCD for display
  Serial.println("Reading Contact: ");   //debugging only
  tempStatusChange(ds_tempF);
  printTemperatures(ds_tempC, ds_tempF); //debugging only
  if(!isFarenheit) {
    lcd.swapUnits();
    isFarenheit = true;
  }
  lcd.clearScreen();
  lcd.printSample(ds_tempF);

  //Send/Print temperature (will only write to ble if user suscribed)
  notificationBuffer[0] = '\0'; //Clear BLE buffer
  uint8_t temp = (uint8_t) ds_tempF; //Convert float to uint8_t
  snprintf(notificationBuffer, sizeof(notificationBuffer) - 1, "T %d", temp);
  bleuart.write(notificationBuffer);

  //Delay between Temperature and Battery readings
  delay(sensor_delay); //15 seconds

  // Get a raw ADC reading
  float vbat_mv = readVBAT();
  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_mv);

  //TODO: Send battery percentage to LCD for display
  //example: displayBattery(somePercentage)
  
  //Send/Print battery percentage (will only write to ble if user subscribed)
  notificationBuffer[0] = '\0'; //Clear BLE buffer
  snprintf(notificationBuffer, sizeof(notificationBuffer) - 1, "B %d", vbat_per);
  bleuart.write(notificationBuffer);
  Serial.println("Battery Percentage: "); //debugging only
  Serial.print(vbat_per); //debugging only
  Serial.println("%"); //debugging only

  //Delay between Temperature and Battery readings
  delay(sensor_delay); //15 seconds
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
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

/**
 * Button triggered interrupt function that reads
 *  temperature in from infrared sensor and prints
 *  to LCD, later will send to app in main loop.
 */
void readInfraredTemp() {

  readInfrared = true;
  mlx_tempC = MLX90614.readObjectTempC();                                        // read temperature in Celsius
  mlx_tempF = ((mlx_tempC * 1.80) + 32);                                         // convert to Fahrenheit
  Serial.println("**************************************");
  Serial.print("Infrared Sensor: ");
  printTemperatures(mlx_tempC, mlx_tempF);
  Serial.println("**************************************");
//  if(isFarenheit) {
//    lcd.swapUnits();
//    isFarenheit = false;
//  }
  lcd.clearScreen();
  lcd.printSample(mlx_tempF);
}

/**
 * Helper function that will change the color of
 *  the neopixel based on the extremities. Colors
 *  include GREEN(Normal), YELLOW(Caution), RED(FEVER) 
 * @param temp read temperature (ambient or infrared)
 */
void tempStatusChange(float temp) {
  if(isFarenheit) {
    if(temp >= FEVER_F) {
      neo.setPixelColor(0, red);
    } else if((temp > NORMAL_MAX_F) or (temp < NORMAL_MIN_F)) {
      neo.setPixelColor(0, yellow);
    } else {
      neo.setPixelColor(0, green);
    }
  } else {
    if(temp >= FEVER_C) {
      neo.setPixelColor(0, red);
    } else if((temp > NORMAL_MAX_C) or (temp < NORMAL_MIN_C)) {
      neo.setPixelColor(0, yellow);
    } else {
      neo.setPixelColor(0, green);
    }
  }
  neo.show();
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
 * Debugging function meant to print out initial message
 *  to console in order to check that Featherboard is ready
 */
void introMessage() {
  Serial.println("Temperature Identification (TempID)");
  Serial.println("-------------------------------------\n");
  Serial.println("Please use TempID Starter App to connect");
}

/**
 * Debugging function meant to see temperature data from
 *  either the contact sensor or infrared sensor.
 */
void printTemperatures(float tempC, float tempF) {
  // Celsius
  Serial.print(tempC);
  Serial.print(" \xC2\xB0");                                                     // print degree symbol                                       
  Serial.print("C  |  ");
  
  // Fahrenheit
  Serial.print(tempF);
  Serial.print(" \xC2\xB0");                                                     // print degree symbol            
  Serial.println("F");
}
