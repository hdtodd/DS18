/* DS18.cpp
   DS18-class thermal sensor object definition

   Created by HDTodd, 2017-July
   OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library
*/

#include "DS18.h"

DS18::DS18(uint8_t pinnumber) : OneWire(pinnumber) {
    _pinnumber = pinnumber;
  };

bool DS18::begin(void) {
  uint8_t addr[8];
  reset();
  reset_search();
  delay(250);
  while ( search(addr) ) {
    Serial.print("Addr: ");
    for (int i=0; i<8; i++) Serial.print(addr[i],HEX);
    Serial.print(" CRC = "); Serial.print(crc8(addr,7),HEX);
    Serial.println();
	       
    if (crc8(addr,7) == addr[7])        // is CRC valid?  If so, check device code
      if ( addr[0]==DS18S20 || addr[0]==DS18B20 || addr[0]==DS1822 ) return true;
  };
  return false;;
  }

/*  set precision for temp conversion to 9, 10, 11, or 12 bits
    addr[] contains OneWire addr of device; resMode is resolution desired
*/
void DS18::setPrecision(uint8_t *addr, uint8_t resMode) {
  uint8_t data[9];
/*  Read in scratchpad, then set the bits for precision we want,
    and finally, write the 3 user bytes back again
*/
  reset();
  select(addr);
  write(readSP);                               // Read Scratchpad
  for (int i = 0; i < 9; i++) data[i] = read(); // we get 9 bytes
  data[4] = (  (data[4] & ~0x60) | (resMode<<5) );  // set the resolution bits
  reset();
  select(addr); 
  write(0x4E);                               // write 3 bytes to scratchpad  
  write_bytes( &data[2],3);
//write(copySP);                             // could copy to EEPROM but won't
  return;
  };

/*  Writes the contents of Scratchpad memory into EEPROM
    so that settings (ID or TH/TL and resolution) will be
    restored after next power-up 
*/
void DS18::saveScratchpad(uint8_t *addr) {
    select(addr);
    write(copySP);                             // copy scratchpad to EEPROM
    return;
  };

/*  reads temperature of sensor at address "addr"
    known to be of type type_s and returns value as a 
    float number, in Celsius scale
*/
float DS18::getTemperature(int type_s, uint8_t *addr, uint8_t *data) {
  uint8_t myResMode;
  select(addr);
  write(convertT);                   // start conversion, NOT IN PARASITE MODE
  for (int delayCount = 0; delayCount<1000 && read_bit()==0; delayCount++) delay(1);
  reset();
  select(addr);    
  write(readSP);                       // Read Scratchpad -- to get temp
  for (int i=0; i<9; i++) data[i] = read();
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
    myResMode = (data[4] & 0x60)>>5;        // get res mode into rh 2 bits as index
    raw &= resMask[myResMode];
  };

  return ( (float)raw / 16.0 );          // return temp in degress Celsius
};
