/* DS18BChar
   Demonstrate the use of the DS18 Class that extends the OneWire Class.
   Characterize connected DS18B20 temperature sensors as a model
   for incorporation into Weather_Station.
   Adapted from OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library

   Modified by HDTodd, July, 2017
*/

#include "OneWire.h"
#include "DS18_Char.h"

#define delayTime 10000		// delay time between sampling loop iterations, 10 sec

DS18 ds18(oneWirePin);		// Create the DS18 object.  Assumed to be
     				// on pin 5 in this example program; see .h
long markTime;			// used to record loop timings
bool firstPass = true;		// first pass through loop does setup, no delay
char *HEX2(uint8_t x);		// used to print hex numbers in 2-digit format always

void setup(void) {
  Serial.begin(9600,SERIAL_8N1);
  if (!ds18.begin()) {
     Serial.println("No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  ds18.reset();
  ds18.reset_search();
  delay(250);			// 250 msec delay req'd after search reset
  Serial.println("Testing program to characterize DS18B20 temperature sensors");
  Serial.print("Connect sensors to Arduino with 3-wire connection.  ");
  Serial.println("NOT in parasitic mode");
  Serial.print("Connect data (yellow/white) to pin "); Serial.print(oneWirePin);
  Serial.println(" with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus");
  Serial.println("\tand reads the temp off each one at 9-, 10-, 11-, and 12-bit");
  Serial.println("\tresolutions -- 0.5, 0.25, 0.125, and 0.0625 degrees C");
  Serial.println();
  }				// end setup()

void loop(void) {
  int i; 
  uint8_t resMode;
  uint8_t data[12], addr[8];
  float celsius;
  
  if ( !ds18.search(addr)) {		// scan for address of next device on OneWire
    if (!firstPass) delay(delayTime);   // no "next device": if not first pass, delay next loop
    firstPass = false; 			// 
    ds18.reset_search();		// reset search, 
    delay(250);				// wait for reset to complete
    for (i=0; i<80; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
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
 
  // Make sure it's a DS18 device; the first ROM byte indicates which chip
  switch (ds18.idDS(addr[0])) {
    case DS18S20:
      Serial.println("  Chip = DS18S20");  // old DS1820
      break;
    case DS18B20:
      Serial.println("  Chip = DS18B20");
      break;
    case DS1822:
      Serial.println("  Chip = DS1822");
      break;
    default:
      Serial.print("Device with code 0x");
      Serial.print(HEX2(addr[0]));
      Serial.println(" is not a DS18x20 family device.");
      return;
  } 

for (resMode=0; resMode<=3; resMode++) {         // We'll cycle through all 4 resolutions
// set precision for conversion
  ds18.reset();
  ds18.select(addr);    
  ds18.setPrecision(addr,resMode);
// sample temp for this probe
  ds18.reset();
  markTime = -millis();			// mark start of conversion, to substract from finish time
  celsius = ds18.getTemperature(addr, data);
  markTime += millis();
// report the probe data, the temp, and the timing for this probe
// we assume that internal non-volatile RAM on the probes has been
// previously set to be used as labels; TL/TH alarms not in use!
  Serial.print("Sensor ID: \'"); Serial.print((char) data[2]); Serial.print((char) data[3]);
  Serial.print("\'\tData = ");
  for ( i = 0; i < 9; i++) { Serial.print(HEX2(data[i])); Serial.print(" "); };
  Serial.print(" CRC=");
  Serial.print(HEX2(OneWire::crc8(data, 8)));
  Serial.print("\tConv time(ms) = "); Serial.print(markTime); 
  Serial.print("  Temperature = "); Serial.print(celsius); Serial.print("C = ");
  Serial.print(celsius * 1.8 + 32.0,1); Serial.println("F");
};                                      // end for(resMode...)
  Serial.println();
}					// end loop()


/*  Returns the value of byte "x" as a 2-digit hexadecimal 
    character array in a 3-byte string suitable for printing 
    String value is NOT PERSISTENT: copy elsewhere if needed */
char* HEX2(uint8_t x) {
  static char str[3];
  static const char hx[16]= {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  str[0] = hx[x/16];
  str[1] = hx[x%16];
  str[2] = 0;
  return str;
};                                        // end HEX2()
