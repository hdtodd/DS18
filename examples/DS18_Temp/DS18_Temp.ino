/* DS18_Temp v2.0

   Demonstrates the use of the DS18 Class library that extends
   the OneWire class of milesburton.com and pjrc.com.

   Report temperatures from all DS18-class probes attached 
   via OneWire communication system to an Arduino master.
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
long markTime;			// used to record loop timings
bool firstPass = true;		// first pass through loop does setup, no delay
char *HEX2(uint8_t x);		// used to print hex numbers in 2-digit format always

void setup(void) {
  Serial.begin(9600);
  if (!ds18.begin()) {
     Serial.println("No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  ds18.reset();
  ds18.reset_search();

  Serial.println("Testing program to demonstrate use of DS18 Class Library to read DS18 temperature sensors");
  Serial.print("Connect sensors to Arduino with 3-wire connection.  ");
  Serial.println("NOT in parasitic mode");
  Serial.print("Connect data (yellow/white) to pin "); Serial.print(oneWirePin);
  Serial.println(" with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus");
  Serial.println("\tand reads the temp off each one at 11-bit resolution = 0.125, degrees C");
  Serial.println();
};				// end setup()

void loop(void) {
  byte i;
  byte type_s;
  byte addr[8], data[12];
  float celsius, fahrenheit;

  if ( !ds18.search(addr)) {		// scan for address of next device on OneWire
    if (!firstPass) delay(delayTime);   // no "next device": if not first pass, delay next loop
    firstPass = false; 			// 
    ds18.reset_search();		// reset search, 
    delay(250);				// wait for reset to complete
    for (i=0; i<80; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
    return;
    };					// end if(!ds18.search())

  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(HEX2(addr[i]));
  };

  if (ds18.crc8(addr, 7) != addr[7]) {
      Serial.println();
      Serial.print("[?DS18_Temp]: CRC is not valid! 0x"); Serial.println(HEX2(ds18.crc8(addr, 7)));
      Serial.println("              Check wiring connections and restart");
      delay(60000);
      return;
  };					// end crc8 check

  // the first address byte indicates which chip type
  switch (addr[0]) {
    case 0x10:
      Serial.print("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.print("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.print("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.print("Device with code 0x");
      Serial.print(HEX2(addr[0]));
      Serial.println("Device is not a known DS18 family device.");
      return;
  } 

  ds18.reset();
  ds18.select(addr);
  ds18.setPrecision(addr,res11);		// set for 11-bit precision, ~350ms
//  ds18.write(0x44, 1);        			// start conversion, with parasite power on at the end
//  delay(1000);     	      			// 1000ms will cover up to 12-bit precision
  // we could do a ds18.depower() here to take power off the data line, 
  //  but the reset will take care of it.
  
  ds18.reset();
  markTime = -millis();				// mark start of conversion for timing
  celsius = ds18.getTemperature(type_s, addr, data);
  markTime += millis();
  Serial.print("\tLabel="); Serial.write(data[2]); Serial.write(data[3]);
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" C = , ");
  Serial.print(celsius * 1.8 + 32.0,2);
  Serial.print(" F");
  Serial.print("\t Conv time = "); Serial.print(markTime); Serial.println(" msec");
  };					// end loop()
  
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
