/**************************************************************************
 * TempID DS18B20 and MXL90614 API
 * Created: 1/04/21
 * Author: Jaocb Schultz
 * Provides: A temperature sensor API for the DS18B20 and MLX90614
 *              temperature sensors
 **************************************************************************/

#include "TemperatureSensors.h"

const int USERBUTTON = 7;                                                         // pin number of pushbutton on board
const int DS18B20_SENSOR_PIN = 9;                                                 // pin on Arduino connected to DS18B20's sensor pin

Adafruit_MLX90614 MLX90614 = Adafruit_MLX90614();                                 // new instance of class
OneWire oneWire(DS18B20_SENSOR_PIN);                                              // new instance of oneWire class
DallasTemperature DS18B20(&oneWire);                                              // pass oneWire reference to DallasTemperature library
DeviceAddress DS18B20_Addr;                                                       // address of DS18B20

// function declarations
//float readInfraredTemp(void);                                                    
//void printTemperatures(float, float);

// global variables
float ds_tempC;                                                                   // DS18B20 temperature in Celsius
float ds_tempF;                                                                   // DS18B20 temperature in Fahrenheit
float mlx_tempC;                                                                  // MLX90614 temperature in Celsius
float mlx_tempF;                                                                  // MLX90614 temperature in Fahrenheit

void TemperatureSensors::initSensors() {
  //Serial.begin(9600);                                                             // initialize serial communication
  DS18B20.begin();                                                                // initialize contact sensor
  MLX90614.begin();                                                               // initialize infrared sensor
  oneWire.reset_search();                                                         // reset oneWire search
  if(!oneWire.search(DS18B20_Addr)){                                              // find address of DS18B20
    Serial.print("Unable to find Address for DS18B20 Sensor"); 
  }
  //pinMode(USERBUTTON, INPUT_PULLUP);                                              // set user button to input
  //attachInterrupt(digitalPinToInterrupt(USERBUTTON), readInfraredTemp, FALLING);  // interrupt for button press
}

// loop for constant reading from contact sensor
double TemperatureSensors::contactSensorLoop() {
  DS18B20.requestTemperatures();                                                  // send command to get readings
  ds_tempC = DS18B20.getTempC(DS18B20_Addr);                                      // read temperature in Celsius
  ds_tempF = DS18B20.toFahrenheit(ds_tempC);                                      // convert to Fahrenheit
  Serial.print("Contact Sensor:  ");
  printTemperatures(ds_tempC, ds_tempF);
  return ds_tempF;
  //delay(2000);                                                                    // wait 2 second
}

// interrupt function for infrared sensor
float TemperatureSensors::readInfraredTemp() {
   mlx_tempC = MLX90614.readObjectTempC();                                        // read temperature in Celsius
   mlx_tempF = ((mlx_tempC * 1.80) + 32);                                         // convert to Fahrenheit
   Serial.print("Infrared Sensor: ");
   printTemperatures(mlx_tempC, mlx_tempF);
   return mlx_tempF;
}

// print temperatures function
void TemperatureSensors::printTemperatures(float tempC, float tempF) {
   // Celsius
   Serial.print(tempC);
   Serial.print(" \xC2\xB0");                                                     // print degree symbol                                       
   Serial.print("C  |  ");
  
   // Fahrenheit
   Serial.print(tempF);
   Serial.print(" \xC2\xB0");                                                     // print degree symbol            
   Serial.println("F");
}
