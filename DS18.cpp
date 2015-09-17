// DS18B20_Characterization
// Characterize connected DS18B20 temperature sensors as a model
//   for incorporation into Weather_Station
// Modified by HDTodd, August, 2015

#include <OneWire.h>
#define oneWirePin 5          // connect data wire to Uno pin 5
                              //  with 4K7 Ohm pullup to VCC 3V3

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

Onewire  ds(oneWirePin);  // on pin 5 (a 4.7K resistor is necessary)

// Temp read conversion times, per Datasheet:
//    9-bit     93.75 ms
//   10-bit    187.5  ms
//   11-bit    375    ms
//   12-bit    750    ms

  int i, delayCount;
  int resMode;
  float celsius, fahrenheit;

int DS18::nextAddr(byte *addr) {   
// Search for the next OneWire device
// Returns 0 and fills in addr[] if found device
// Returns 1 and fills in addr[] if reset scan at beginning
//                               and found device
// Returns -1 if CRC error in reading device ROM
  if ( !ds.search(addr)) {        // scan for address of next device on OneWire
    if (!firstPass) delay(10000);
    firstPass = false;
    ds.reset_search();             // or reset list and start new scan
    delay(250);
    return 1;
  };  
  if (OneWire::crc8(addr, 7) != addr[7]) {    // confirm CRC of address
    Serial.println("CRC is not valid!");
    Serial.print("ROM =");           // report the addresss of the next device
    for( i = 0; i < 8; i++) {Serial.write(' '); Serial.print(addr[i], HEX); };
    return -1;
  };
  return 0;
}

int DS18::verifyType(byte *addr) {
  // Make sure the device at *addr is a DS18 device
  //   the first ROM byte indicates which chip type
  // Returns 0 if DS18B20 or 22
  //         1 if DS18S20 (old chip)
  //        -1 if not a DS18-type device
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
      type_s = -1;
      break;
  };
  return type_s;
}

void DS18::setPrecision(byte *addr, resModes resMode) {
//  set precision for temp conversion to 9, 10, 11, or 12 bits
//  addr[] contains OneWire addr of device; resMode is resolution desired
//       first, read in scratchpad
//       then set the bits for precision we want
//       then write the 3 user bytes back again
  ds.reset();
  ds.select(addr);    
  ds.write(readSP);                               // Read Scratchpad
  for ( i = 0; i < 9; i++) data[i] = ds.read(); // we get 9 bytes
  data[4] = (  (data[4] & ~0x60) | (resMode<<5) );  // set the resolution bits
  ds.reset();
  ds.select(addr); 
  ds.write(writeSP);                               // write 3 bytes to scratchpad 
  ds.write_bytes( &data[2],3);
}

void DS18::saveScratchpad(byte *addr) {
//  Writes the contents of Scratchpad memory into EEPROM
//    so that settings (ID or TH/TL and resolution) will be
//    restored after next power-up 
  ds.reset();
  ds.select(addr);    
  ds.write(saveSP);                             // and write that scratchpad to EEPROM
};


float DS18::getTemperature(int type_s, byte *addr) {
//  reads temperature of sensor at address "addr"
//    known to be of type type_s
//  and returns value as a float number, in Celsius scale
  ds.reset();
  ds.select(addr);
  ds.write(sampleTemp);        // start conversion, NOT IN PARASITE MODE
  for (delayCount = 0; delayCount<1000 && ds.read_bit()==0; delayCount++) delay(1);
  
  ds.reset();
  ds.select(addr);    
  ds.write(readSP);                       // Read Scratchpad -- to get temp
  for ( i=0; i<9; i++) data[i] = ds.read();
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {                           // older device format
    raw = raw << 3;                       // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {                                // DS18B style device
                                          // mask data to resolution length
    resMode = (data[4] & 0x60)>>5;        // get res mode into rh 2 bits as index
    raw &= resMask[resMode];
  };

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  return celsius;
}
