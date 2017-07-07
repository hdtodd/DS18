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

/* Discover any DS18-class devices on the OneWire buss.  Library
   recognizes {DS18S20, DS18B20, DS1822} and knows how to set 
   resolution and read temps for those devices.
   Returns (bool)true if there is at least one DS18-class device on 
   the buss or (bool)false if there isn't.
*/
bool DS18::begin(void) {
  uint8_t addr[8];
  reset();
  reset_search();
  delay(250);                  // required for search reset
  while ( search(addr) ) {
    // if CRC is valid and device code is one we know, we found a DS18; return "true"
    if ( (crc8(addr,7)  == addr[7]) && 
	 (idDS(addr[0]) != DSNull ) && 
         (idDS(addr[0]) != DSUnkwn) ) return true;
    };                         // end while
  // if we got here, no device found
  return false;
};                             // end begin()

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
  data[4] = ( (data[4] & ~0x60) | (resMode<<5) );  // set the resolution bits
  reset();
  select(addr); 
  write(writeSP);                    // write 3 bytes to scratchpad  
  write_bytes( &data[2],3);
//write(copySP);                     // could copy to EEPROM but won't
  return;
  };

/*  Writes the contents of Scratchpad memory into EEPROM
    so that settings (ID or TH/TL and resolution) will be
    restored after next power-up 
*/
void DS18::saveScratchpad(uint8_t *addr) {
    select(addr);
    write(copySP);                   // copy scratchpad to EEPROM
    return;
  };

/*  reads temperature of sensor at address "addr"
    and returns value as a float number, in Celsius scale
*/
float DS18::getTemperature(uint8_t *addr, uint8_t *data) {
  uint8_t myResMode;
  select(addr);
  write(convertT);                   // start conversion, NOT IN PARASITE MODE
                                     // Per OneWire docs, change that to 
                                     // write(converT,1) if DS18's are in parasite mode
                                     // but beware of possible device burnout
  // now wait for device ready or timeout
  for (int delayCount = 0; delayCount<1000 && read_bit()==0; delayCount++) delay(1);
  reset();
  select(addr);    
  write(readSP);                     // Read Scratchpad to get temp reading
  for (int i=0; i<9; i++) data[i] = read();
  // Convert the data to temperature in float format from the 
  // DS18 internal representation in a 16-bit integer
  int16_t raw = (data[1] << 8) | data[0];
  /* This code supports just two formats for the temperature data returned.
     If other formats are needed, add to typeDS enumeration,
     and modify the following "if" to be a "switch" that includes code to
     handle the new format.
  */
  if (idDS(addr[0])==DS18S20) {        // older device format
    raw = raw << 3;                    // 9 bit resolution default
    if (data[7] == 0x10) {             // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {                             // mask data to resolution length
    myResMode = (data[4] & 0x60)>>5;   // get res mode into rh 2 bits as index
    raw &= resMask[myResMode];
  };

  return ( (float)raw / 16.0 );        // return temp in degress Celsius
};

typeDS DS18::idDS(uint8_t code) {
  uint8_t id;
  for (id=(uint8_t)DSNull; id<=(uint8_t)DSUnkwn; id++) 
    if (code==listDS18s[id].devCode) return ((typeDS)id);
  return DSUnkwn;
};				       // end idDS()

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

// Temp conversion routines
float FtoC(float fahrenheit) {
  return ( (fahrenheit - 32.0)/1.8 );
};

float CtoF(float celsius) {
  return ( celsius*1.8 + 32.0 );
};
