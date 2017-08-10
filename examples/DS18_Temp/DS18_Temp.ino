/* DS18_Temp v2.0

   Demonstrates the use of the DS18 Class library that extends
   the OneWire class of milesburton.com and pjrc.com.

   Report temperatures from all DS18-class probes attached 
   to an Arduino master via OneWire communication system.
   Adapted from OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library

   Modified by HDTodd, July, 2017, to use the DS18 library.
*/

#include "OneWire.h"
#include "DS18_Temp.h"

#define delayTime 10000		// delay time between sampling loop iterations, 10 sec

DS18 ds18(oneWirePin);		// Create the DS18 object.  Assumed to be
     				// on pin 5 in this example program; see .h
bool firstPass = true;		// first pass through loop does setup, no delay
long markTime, loopTime=0;		// used to record loop timings

void setup(void) {
  Serial.begin(9600);
  Serial.println("Testing program to demonstrate use of DS18 Class Library to read DS18 temperature sensors");
  Serial.println("Program scans through all DS18\'s on the OneWire bus");
  Serial.println("  and reads the temp off each one at 11-bit resolution = 0.125, degrees C");
  Serial.println();
  Serial.println("Connect sensors to Arduino with 3-wire connection, NOT in parasitic mode");
  Serial.print(  "Connect data (yellow/white) to pin "); 
  Serial.print(  oneWirePin);
  Serial.println("  with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println();
  if (!ds18.begin()) {
     Serial.println("[?Temp]: No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  ds18.reset();
  ds18.reset_search();
  delay(250);			// search resets require a 250 msec delay afterward
};				// end setup()

void loop(void) {
  uint8_t addr[8], data[9];
  float celsius;

  if ( !ds18.search(addr) ) {		// scan for address of next device on OneWire
    // no next device; summarize this scan  and prepare for next scan loop
    for (int i=0; i<49; i++) Serial.write(' ');
    Serial.print("Total loop conversion time: ");
    Serial.print(loopTime); Serial.println(" msec");
    for (int i=0; i<85; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
    loopTime=0;
    firstPass = false;
    ds18.reset_search();		// reset search, 
    delay(delayTime);   		// must be >= 250 msec for search reset
    return;
    };					// end if(!ds18.search())

// Validate address with a CRC check
  if (ds18.crc8(addr, 7) != addr[7]) {
      Serial.println();
      Serial.print("[?DS18_Temp]: CRC is not valid! 0x"); Serial.println(HEX2(ds18.crc8(addr, 7)));
      Serial.println("              Check wiring connections and restart");
      delay(60000);
      return;
  };					// end crc8 check

// Make sure it's a DS18 device; the first address byte indicates which chip
  if ( (ds18.idDS(addr[0])==DSNull) || (ds18.idDS(addr[0])==DSUnkwn) ) {
    Serial.print("Device with code 0x"); Serial.print(HEX2(addr[0]));
    Serial.println(" is not a known DS18 family device.");
    delay(60000);
    return;
    };

// Now take its temperature and time the process
  ds18.reset();
  ds18.select(addr);
  ds18.setPrecision(addr,res11);		// set for 11-bit precision, ~350ms
  ds18.reset();
  markTime = -millis();				// mark start of conversion for timing
  celsius = ds18.getTemperature(addr, data, true);    // returns when conversion complete
  markTime += millis();
  loopTime += markTime;
  Serial.print("Label='"); Serial.write(data[2]); Serial.write(data[3]);
  Serial.print("'   Chip = "); Serial.print(listDS18s[ds18.idDS(addr[0])].devName);
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" C = , ");
  Serial.print(CtoF(celsius),2);
  Serial.print(" F");
  Serial.print("\t Conv time = "); Serial.print(markTime); Serial.println(" msec");

  };					// end loop()
  