/* DS18_Labeler v2.0
   Demonstrate the use of the DS18 Class for the Arduino that extends 
   the OneWire Class of milesburton.com and pjrc.com.

   Label connected DS18B20 temperature sensors by writing a 2-char code
     into their internal EEPROM memory ("TL/TH") so that they can be
     identified by software when collecting temperature data.
   This program labels multiple connected sensors at one sitting
     Hold one in your hand to identify it by watching for the sensor with
     increasing temperature, then label that sensor when given the opportunity

  DS18 Class library dapted from OneWire DS18S20, DS18B20, DS1822 Temperature Example
    http://www.pjrc.com/teensy/td_libs_OneWire.html
    http://milesburton.com/Dallas_Temperature_Control_Library

  Written by HDTodd, August, 2015
  Modified by HDTodd, July, 2017, to use the DS18 library.
*/

#include "OneWire.h"
#include "DS18_Labeler.h"

#define delayTime 10000		// delay time between sampling loop iterations, 10 sec

DS18 ds18(oneWirePin);  		// on pin 5 (a 4.7K resistor is necessary)
long markTime;
boolean firstPass=true;
boolean doneLabeling=false;

void setup(void) {
  Serial.begin(9600);
  Serial.println("\nTesting program to label DS18 temperature sensors and demonstrate DS18 library.");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus.");
  Serial.println("  Reads the temp off each one at 10-bit resolution");
  Serial.println("  and offers the opportunity to label each one.");
  Serial.println();
  Serial.println("Hold one sensor and watch for the temperature to rise, then");
  Serial.println("  label that sensor and mark the physical probe accordingly.");
  Serial.println("Take each sensor in turn until you\'ve labeled the set.");
  Serial.println("BE SURE THAT THE INPUT TO THIS PROGRAM IS SET TO SEND <NEWLINE>");
  Serial.println("  when you're replying to the response prompts.\n");
  Serial.println();
  Serial.println("Connect sensors to Arduino with 3-wire connection, NOT in parasitic mode");
  Serial.print(  "Connect data (yellow/white) to pin "); 
  Serial.print(  oneWirePin);
  Serial.println("  with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println();
  Serial.println("First we'll scan the OneWire bus to identify devices.");
  Serial.println("Then we'll report again on each device and give you a chance");
  Serial.println("  to label each one.  Labels are limited to TWO CHARACTERS");
  if (!ds18.begin()) {
     Serial.println("[?Labeler]: No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  ds18.reset();
  ds18.reset_search();
  delay(250);			// 250 msec delay req'd after search reset

};  		      	        // end setup()

void loop(void) {
  uint8_t data[9], addr[8];
  float celsius;
  char buf[100];
  
  if ( !ds18.search(addr) ) {		// scan for address of next device on OneWire
    // no next device; prepare for next scan loop
    for (int i=0; i<120; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
    ds18.reset_search();		// reset search, 
    delay(250);				// must wait at least 250 msec for reset search
    return;
  };

  Serial.print("ROM =");		// report the address of the next device
  for(int i = 0; i < 8; i++) {Serial.write(' '); Serial.print(HEX2(addr[i])); };

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

// Set precision for conversion here
  ds18.setPrecision(addr,res10);

  ds18.reset();
  ds18.select(addr);
  markTime = -millis();   // mark start of conversion
  celsius = ds18.getTemperature(addr,data);
  markTime += millis();
  Serial.print("Sensor ID: \'"); Serial.print((char) data[2]); Serial.print((char) data[3]);
  Serial.print("\'  Conv time = "); Serial.print(markTime); Serial.print(" msec");
  Serial.print("  Temperature = "); 
  Serial.print(CtoF(celsius)); Serial.println(" F");

  if (!doneLabeling) {
    if (Serial.available()>0) Serial.readBytesUntil('\n', buf, 100);  // clear buffer
    Serial.print("Do you want to label this sensor? (y or n, or q to quit) ");
    while (Serial.available()<2) {};
    Serial.readBytesUntil('\n', buf, 100);
    switch (tolower(buf[0])) {
      case 'y':
         Serial.print("Type two characters to label this sensor: ");
             while (Serial.available()<3) {};
         Serial.readBytesUntil('\n', buf, 100);
         data[2] = buf[0];
         data[3] = buf[1]; 
         Serial.print("labeling with \'"); Serial.print(buf[0]); Serial.print(buf[1]);
         Serial.println("\'");
         ds18.reset();
         ds18.select(addr);
         ds18.write(writeSP);                               // write 3 bytes to scratchpad  
         ds18.write_bytes( &data[2],3 );
         ds18.reset();
         ds18.select(addr);
         ds18.write(copySP);                               // and write that scratchpad to EEPROM         
         break;
      case 'n':       
        break;
      case 'q':
        doneLabeling = true;
        break;
      default:   
        break;
    };
  };
  
  Serial.println();
  firstPass = false;  
}
