/**************************************************************************
 * TempID DS18B20 and MXL90614 API Header
 * Created: 1/04/21
 * Author: Jaocb Schultz
 * Provides: A temperature sensor API for the DS18B20 and MLX90614
 *              temperature sensors
 **************************************************************************/

#ifndef TEMPERATURESENSORS_H
#define TEMPERATURESENSORS_H

 //Inclusions
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>


class TemperatureSensors{

public:

/*
*   initSensors
*       initializes sensors and locates address of 
*       DS18B20 connected
*/
void initSensors();

/*
*   contactSensorLoop
*       loop to continuously take readings from
*       the DS18B20 contact sensor
*       NOTE: will probably have to be moved to a while loop
*             in main
*/
double contactSensorLoop();

/*
*   readInfraredTemp
*       interrupt function triggered by the user button
*       to take a reading from the MLX90614 infrared sensor
*/ 
float readInfraredTemp();

/*
*   printTemperatures
*       prints the temperature readings from DS18B20 and
*       MXL90614 sensors
*/ 
void printTemperatures(float tempC, float tempF);

};

 #endif //TEMPERATURESENSORS_H
