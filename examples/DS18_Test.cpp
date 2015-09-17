// DS18B20_Characterization
// Characterize connected DS18B20 temperature sensors as a model
//   for incorporation into Weather_Station
// Modified by HDTodd, August, 2015

#include <DS18.h>
#define oneWirePin 5          // connect data wire to Uno pin 5
                              //  with 4K7 Ohm pullup to VCC 3V3

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

DS18 ds(oneWirePin);  // on pin 5 (a 4.7K resistor is necessary)

// Temp read conversion times, per Datasheet:
//    9-bit     93.75 ms
//   10-bit    187.5  ms
//   11-bit    375    ms
//   12-bit    750    ms

long markTime;

void setup(void) {
  Serial.begin(115200);
  ds.init();

  Serial.println("Testing program to characterize DS18B20 temperature sensors");
  Serial.print("Connect sensors to Arduino with 3-wire connection.  ");
  Serial.println("NOT in parasitic mode");
  Serial.print("Connect data (yellow/white) to pin "); Serial.print(oneWirePin);
  Serial.println(" with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus");
  Serial.println("\tand reads the temp off each one at 9-, 10-, 11-, and 12-bit");
  Serial.println("\tresolutions -- 0.5, 0.25, 0.125, and 0.0625 degrees C");
  Serial.println();
}

void loop(void) {
  int i, delayCount;
  int resMode;
  uint8_t present = 0;
  uint8_t type_s;
  uint8_t data[12], addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {         // scan for address of next device on OneWire
    if (!firstPass) delay(10000);
    firstPass = false;
    for (i=0; i<120; i++) Serial.print('-'); Serial.println(); 
    Serial.println("NEXT SCAN");
    ds.reset_search();             // or reset list and start new scan
    delay(250);
    return;
  }
  
  Serial.print("ROM =");           // report the addresss of the next device
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
for (resMode=0; resMode<4; resMode++) {         // We'll cycle through all 4 resolutions
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
  Serial.print("\'\tData = ");
  for ( i = 0; i < 9; i++) { Serial.print(data[i], HEX); Serial.print(" "); };
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.print("\tConv delay loop "); Serial.print(delayCount); 
  Serial.print("\tConv time(ms) = "); Serial.print(-markTime); 

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
  Serial.print("  Temperature = "); Serial.print(celsius); Serial.print("C = ");
  Serial.print(fahrenheit); Serial.println("F");
}

  Serial.println();
}
