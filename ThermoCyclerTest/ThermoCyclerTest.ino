/***************************************************
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include "Adafruit_MAX31855.h"

// Default connection is using software SPI, but comment and uncomment one of
// the two examples below to switch between software SPI and hardware SPI:

// Example creating a thermocouple instance with software SPI on any three
// digital IO pins.
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5
#define heatPin 11
#define fanPin 10
// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Example creating a thermocouple instance with hardware SPI
// on a given CS pin.
//#define MAXCS   10
//Adafruit_MAX31855 thermocouple(MAXCS);

void setup() {
  Serial.begin(9600);
  pinMode(11,OUTPUT);
  while (!Serial) delay(1); // wait for Serial on Leonardo/Zero, etc

  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
}
boolean reachTemp(double desiredTemp){
  double currentTemp = thermocouple.readCelsius();
  int count;
  while((desiredTemp > currentTemp)and (count != 3)){
           Serial.print("C = ");
     Serial.println(currentTemp);
     currentTemp = thermocouple.readCelsius();
     digitalWrite(heatPin, HIGH);
     delay(1000);
     if (desiredTemp > currentTemp){
      count += 1;
      Serial.println("count is" + count);
     }
  }
  return true;
}
boolean holdConstantTemp(long duration, double idealTemp){
    double currentTemp;
   unsigned long startTime = millis();
  long timeElapsed = millis() - startTime;
  while (timeElapsed < duration){
         Serial.print("C = ");
     Serial.println(currentTemp);
  currentTemp = thermocouple.readCelsius();
  if (currentTemp < idealTemp){
    digitalWrite(heatPin, HIGH);
    delay(500);
    digitalWrite(heatPin, LOW);
  }
  else if (currentTemp > (idealTemp+0.5)){
    digitalWrite(fanPin,HIGH);
    delay(500);
    digitalWrite(fanPin,LOW);
  }
  delay(210);
  timeElapsed = millis() - startTime;
}
  return true;
}
void loop() {
  reachTemp(45.0);
  Serial.println("temp reached! Holding...");
  holdConstantTemp(30000000, 43.0);
}
