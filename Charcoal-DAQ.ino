////////////////////
// Timing library //
////////////////////
#include "SimpleTimer.h"
SimpleTimer timer;


////////////////////////////////
// Dallas Temperature Library //
////////////////////////////////
#include "OneWire.h"
#include "DallasTemperature.h"
#define ONE_WIRE_BUS 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress T1 = { 0x28, 0x8A, 0xB0, 0xC3, 0x06, 0x00, 0x00, 0x14 };
DeviceAddress T2 = { 0x28, 0x28, 0xE7, 0xC4, 0x06, 0x00, 0x00, 0x64 };
DeviceAddress T3 = { 0x28, 0x0A, 0xF5, 0xC4, 0x06, 0x00, 0x00, 0x43 };
DeviceAddress T4 = { 0x28, 0xBE, 0x8F, 0xC4, 0x06, 0x00, 0x00, 0x48 };
DeviceAddress T5 = { 0x28, 0x85, 0x25, 0xC5, 0x06, 0x00, 0x00, 0x5C };
float temp1, temp2, temp3, temp4, temp5;

// Records all Dallas Temperatures
void getDallasTemps(){
  temp1 = sensors.getTempC(T1);
  temp2 = sensors.getTempC(T2);
  temp3 = sensors.getTempC(T3);
  temp4 = sensors.getTempC(T4);
  temp5 = sensors.getTempC(T5);
  sensors.requestTemperatures();
}


///////////////////////////////////u
// Adafruit Thermocouple Library //
///////////////////////////////////u
#include "max6675.h"
#define THERMOCLK 9
#define THERMODO  4
#define THERMO1CS 5
#define THERMO2CS 6
MAX6675 thermocouple1(THERMOCLK, THERMO1CS, THERMODO);
MAX6675 thermocouple2(THERMOCLK, THERMO2CS, THERMODO);
float thermo1, thermo2;

void getThermocoupleTemps(){
  thermo1 = thermocouple1.readCelsius();
  thermo2 = thermocouple2.readCelsius();
}


/////////////////
// DustTrak II //
/////////////////
#define DUSTTRAKPIN 1

void getDustTrak(){
  dustTrak = ((float) analogRead(DUSTTRAKPIN) - 10) / 1023 * 20);
}


///////////////
// Bacharach //
///////////////
#include "SoftwareSerial.h"
#define SOFTTX 11
#define SOFTRX 10
SoftwareSerial softSerial(SOFTRX, SOFTTX);
float O2, CO, Tair, NO, NO2, NOx, SO2;

void getBacharach(){
  if (softSerial.available() > 25){
    // Starting read
    char c = softSerial.read();
    int i = 0;
    while (c != '\n'){
      String d = String();
      while(softSerial.peek() != ','){
        d += (char) softSerial.read();
      }
      switch (i){
        case 9:
	  O2 = d.toFloat();
	  break;
        case 10:
	  CO = d.toFloat();
	  break;
        case 14:
	  Tair = d.toFloat();
	  break;
        case 17:
	  NO = d.toFloat();
	  break;
        case 18:
	  NO2 = d.toFloat();
	  break;
        case 19:
	  NOx = d.toFloat();
	  break;
        case 20:
	  SO2 = d.toFloat();
	  break;
      }
      // clear the newline
      softSerial.read();
    }
  }
}


///////////////////////////////////
// Setup, Loop, and Log function //
/////////////////////////////////// 
void setup(void) {
  Serial.begin(9600);
  
  // Start up the Dallas Temperature Sensors
  sensors.begin();
  sensors.setResolution(T1, 12);
  sensors.setResolution(T2, 12);
  sensors.setResolution(T3, 12);
  sensors.setResolution(T4, 12);
  sensors.setResolution(T5, 12);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  timer.setInterval(1000, getDallasTemps);

  // Set up the thermocouples
  timer.setInterval(1000, getThermocoupleTemps);

  // Set up the DustTrak
  timer.setInterval(1000, getDustTrak);

  // Set up Bacharach
  softSerial.begin(19200);
  while(softSerial.peek() != '\n'){
    softSerial.read();
  }
  timer.setInterval(1000, getBacharach);

  // Log the data
  Serial.print("time,temp1,temp2,temp3,temp4,temp5,thermo1,thermo2,");
  Serial.println("dustTrak,O2,CO,Tair,NO,NO2,NOx,SO2");
  timer.setInterval(1000, log);
}

void loop(void) {
  timer.run();
}

void log(){
  Serial.print(((float) millis())/1000);
  Serial.print(',');
  Serial.print(temp1);
  Serial.print(',');
  Serial.print(temp2);
  Serial.print(',');
  Serial.print(temp3);
  Serial.print(',');
  Serial.print(temp4);
  Serial.print(',');
  Serial.print(temp5);
  Serial.print(',');
  Serial.print(thermo1);
  Serial.print(',');
  Serial.print(thermo2);
  Serial.print(',');
  Serial.print(dustTrak);
  Serial.print(',');
  Serial.print(O2);
  Serial.print(',');
  Serial.print(CO);
  Serial.print(',');
  Serial.print(Tair);
  Serial.print(',');
  Serial.print(NO);
  Serial.print(',');
  Serial.print(NO2);
  Serial.print(',');
  Serial.print(NOx);
  Serial.print(',');
  Serial.print(SO2);
  Serial.println();
}


////////////////////////////////////////////////////
// Reference to find Dallas Temperature Addresses //
////////////////////////////////////////////////////
void discoverOneWireDevices(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  Serial.print("Looking for 1-Wire devices...\n\r");
  while(oneWire.search(addr)) {
    Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return;
    }
  }
  Serial.print("\n\r\n\rThat's it.\r\n");
  oneWire.reset_search();
  return;
}

