/* DS18_Char v2.0
   Demonstrate the use of the DS18 Class that extends 
   the OneWire Class of milesburton.com and pjrc.com.

   Characterize connected DS18-class temperature sensors attached
   to an Arduino master via OneWire communication system.
   Adapted from OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library

   Modified by HDTodd, July, 2017, to use the DS18 library.
*/

#include "OneWire.h"
#include "DS18_Char.h"

#define delayTime 10000		// delay time between sampling loop iterations, 10 sec

DS18 ds18(oneWirePin);		// Create the DS18 object.  Assumed to be
     				// on pin 5 in this example program; see .h
long markTime, loopTime=0;	// used to record loop timings

void setup(void) {
  Serial.begin(9600,SERIAL_8N1);
  Serial.println("Testing program to demonstrate use of DS18 Class Library to read DS18 temperature sensors");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus");
  Serial.println("  and times reading the temp off each one at 9-, 10-, 11-, and 12-bit");
  Serial.println("  resolutions -- 0.5, 0.25, 0.125, and 0.0625 degrees C");
  Serial.println();
  Serial.println("Connect sensors to Arduino with 3-wire connection, NOT in parasitic mode");
  Serial.print(  "Connect data (yellow/white) to pin "); 
  Serial.print(  oneWirePin);
  Serial.println("  with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println();
  if (!ds18.begin()) {
     Serial.println("[?Char]: No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  ds18.reset();
  ds18.reset_search();
  delay(250);			// 250 msec delay req'd after search reset
  };				// end setup()

void loop(void) {
  int i; 
  uint8_t resMode;
  uint8_t data[9], addr[8];
  float celsius;
  
  if ( !ds18.search(addr)) {		// scan for address of next device on OneWire
    // no next device; prepare for next scan loop
    for (i=0; i<113; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
    ds18.reset_search();		// reset search, 
    delay(delayTime);			// must wait at least 250 msec for reset search
    return;
  }
  
  Serial.print("ROM =");		// report the address of the next device
  for( i = 0; i < 8; i++) {Serial.write(' '); Serial.print(HEX2(addr[i])); };

  if (ds18.crc8(addr, 7) != addr[7]) {    // confirm CRC of address
    Serial.println();
    Serial.print("CRC is not valid! 0x"); Serial.println(HEX2(OneWire::crc8(addr, 7)));
    delay(60000);
    return;
    }
 
// Make sure it's a DS18 device; the first address byte indicates which chip
  Serial.print("   Chip = "); Serial.println(listDS18s[ds18.idDS(addr[0])].devName);
  if ( (ds18.idDS(addr[0])==DSNull) || (ds18.idDS(addr[0])==DSUnkwn) ) {
    Serial.print("Device with code 0x"); Serial.print(HEX2(addr[0]));
    Serial.println(" is not a known DS18 family device.");
    delay(60000);
    return;
    };

  for (resMode=0; resMode<=3; resMode++) {         // We'll cycle through all 4 resolutions
// Set precision for conversion
    ds18.reset();
    ds18.select(addr);    
    ds18.setPrecision(addr,resMode);
    ds18.reset();
    markTime = -millis();			// mark start of conversion, to substract from finish time
    celsius = ds18.getTemperature(addr, data);  // sample temp for this probe
    markTime += millis();

/* Report the probe data, the temp, and the timing for this probe
   We assume that internal non-volatile RAM on the probes has been
   previously set to be used as labels; TL/TH alarms not in use!
   Otherwise modify code below to write 8 probe address bytes as HEX2.
*/
    Serial.print("Sensor ID: \'"); Serial.print((char) data[2]); Serial.print((char) data[3]);
    Serial.print("\'\tData = ");
    for ( i = 0; i < 9; i++) { Serial.print(HEX2(data[i])); Serial.print(" "); };
    Serial.print(" CRC=");
    Serial.print(HEX2(OneWire::crc8(data, 8)));
    Serial.print("  Prec="); Serial.print(ds18.getPrecision(addr));
    Serial.print("  Conv time(ms) = "); 
    if (markTime<100) Serial.write(' '); Serial.print(markTime); 
    Serial.print("  Temperature = "); Serial.print(celsius); Serial.print("C = ");
    Serial.print(celsius * 1.8 + 32.0,1); Serial.println("F");
    };                                        // end for(resMode...)
  Serial.println();
  };					// end loop()


