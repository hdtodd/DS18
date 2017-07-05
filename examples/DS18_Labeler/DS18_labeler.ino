// DS18B20_Labeler
// Label connected DS18B20 temperature sensors by writing a 2-char code
//   into their internal memory ("TL/TH") so that they can be
//   identified programmatically
// This program labels multiple connected sensors at one sitting
//   Hold one in your hand to identify it by watching for the sensor with
//   increasing temperature, then label that sensor when given the opportunity
// Written by HDTodd, August, 2015, from a modified version of the 
//   DS18B20 characterization program


#include <OneWire.h>
#define oneWirePin 5          // connect data wire to Uno pin 5
                              //  with 4K7 Ohm pullup to VCC 3V3

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(oneWirePin);  // on pin 5 (a 4.7K resistor is necessary)
typedef enum {res9=0b00, res10=0b01, res11=0b10, res12=0b11} resModes;
const uint16_t resMask[] = {~7, ~3, ~1, ~0};   // masks to truncate raw data
// Temp read conversion times, per Datasheet:
//    9-bit     93.75 ms
//   10-bit    187.5  ms
//   11-bit    375    ms
//   12-bit    750    ms

long markTime;
boolean firstPass=true;
boolean doneLabeling=false;

void setup(void) {
  Serial.begin(9600);
  Serial.println();
  delay(1000);
  Serial.println("\n\nTesting program to label DS18B20 temperature sensors");
  Serial.print("Connect sensors to Arduino with 3-wire connection.  ");
  Serial.println("NOT in parasitic mode");
  Serial.print("Connect data (yellow/white) to pin "); Serial.print(oneWirePin);
    Serial.println(" with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus");
  Serial.println("\tand reads the temp off each one at 10-bit resolution");
  Serial.println("\tand offers the opportunity to label each one.");
  Serial.println();
  Serial.println("Hold one sensor and watch for the temperature to rise, then");
  Serial.println("\tlabel that sensor and mark the physical probe accordingly.");
  Serial.println("\tTake each sensor in turn until you\'ve labeled the set.");
  Serial.println("BE SURE THAT THE INPUT TO THIS PROGRAM IS SET TO SEND <NEWLINE>");
  Serial.println("\twhen you're replying to the response prompts.\n");
}

void loop(void) {
  int i, delayCount;
  int resMode;
  uint8_t present = 0;
  uint8_t type_s;
  uint8_t data[12], addr[8];
  float celsius, fahrenheit;
  char ch, buf[100];
  
  if (firstPass) {
    Serial.println("First we'll scan the OneWire bus to identify devices.");
    Serial.println("Then we'll report again on each device and give you a chance");
    Serial.println("\tto label each one.  Labels are limited to TWO CHARACTERS");
  };
  
  if ( !ds.search(addr)) {         // scan for address of next device on OneWire
    if (!firstPass) delay(10000);
    for (i=0; i<120; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
    ds.reset_search();             // or reset list and start new scan
    delay(250);
    return;
  }
  
  Serial.print("\nROM =");           // report the address of the next device
  for( i = 0; i < 8; i++) {Serial.write(' '); Serial.print(addr[i], HEX); };

  if (OneWire::crc8(addr, 7) != addr[7]) {    // confirm CRC of address
      Serial.println("CRC is not valid!");
      return;
  }
 
  // Make sure it's a DS18 device; the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.print("Device with code 0x");
      Serial.print(addr[0],HEX);
      Serial.println(" is not a DS18x20 family device.");
      return;
  } 

//[hdt]  set precision for conversion here
//       first, read in scratchpad
//       then set the bits for precision we want
//       then write the 3 user bytes back again
  resMode = res10;
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);                               // Read Scratchpad
  for ( i = 0; i < 9; i++) data[i] = ds.read(); // we get 9 bytes
  data[4] = (  (data[4] & ~0x60) | (resMode<<5) );  // set the resolution bits
  ds.reset();
  ds.select(addr); 
  ds.write(0x4E);                               // write 3 bytes to scratchpad  
  ds.write_bytes( &data[2],3);
//  ds.write(0x48);                               // and write that scratchpad to EEPROM
//[/hdt]

  ds.reset();
  ds.select(addr);
  markTime = millis();   // mark start of conversion
  ds.write(0x44);        // start conversion, NOT IN PARASITE MODE
  for (delayCount = 0; delayCount<1000 && ds.read_bit()==0; delayCount++) delay(1);
  markTime -= millis();
  
  ds.reset();
  ds.select(addr);    
  ds.write(0xBE);                       // Read Scratchpad -- to get temp
  for ( i=0; i<9; i++) data[i] = ds.read();
  Serial.print("Sensor ID: \'"); Serial.print((char) data[2]); Serial.print((char) data[3]);
  Serial.print("\'\tConv time = "); Serial.print(-markTime); Serial.print("ms");

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {                           // older device format
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {                                // mask data to resolution length
    resMode = (data[4] & 0x60)>>5;        // get res mode into rh 2 bits as index
    raw &= resMask[resMode];
  };

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = "); 
  Serial.print(fahrenheit); Serial.println("F");

  if (!doneLabeling) {
    if (Serial.available()>0) Serial.readBytesUntil('\n', buf, 100);  // clear buffer
    Serial.print("Do you want to label this sensor? (y or n, or q to quit) ");
    while (Serial.available()<2) {};
    Serial.readBytesUntil('\n', buf, 100); ch = buf[0];
    
    if (ch=='Y') ch='y'; if (ch=='N') ch='n'; if (ch=='Q') ch='q';
    switch (ch) {
      case 'y':
         Serial.print("Type two characters to label this sensor: ");
             while (Serial.available()<3) {};
         Serial.readBytesUntil('\n', buf, 100);
         data[2] = buf[0];
         data[3] = buf[1]; 
         Serial.print("labeling with \'"); Serial.print(buf[0]); Serial.print(buf[1]);
         Serial.println("\'");
         ds.reset();
         ds.select(addr);
         ds.write(0x4E);                               // write 3 bytes to scratchpad  
         ds.write_bytes( &data[2],3 );
         ds.reset();
         ds.select(addr);
         ds.write(0x48);                               // and write that scratchpad to EEPROM         
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