/* DS18.cpp
   DS18-class thermal sensor object definition

   Created by HDTodd, 2017-July
   OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library
*/

#include "DS18.h"
int initTime;                // times concurrent conversion

DS18::DS18(uint8_t pinnumber) : OneWire(pinnumber) {
    _pinnumber = pinnumber;
  };

/* Discover any DS18-class devices on the OneWire buss.  Library
   recognizes {DS18S20, DS18B20, DS1822} and knows how to set 
   resolution and read temps for those devices.
   Returns (boolean)true if there is at least one DS18-class device on 
   the buss or (boolean)false if there isn't.
*/
boolean DS18::begin(void) {
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
//write(copySP);                     // uncomment to make precision permanent
  return;
  };


/* Returns the precision of conversion being used by the
   DS18 device at "addr".  Precision is returned as the
   enumerated type identifier resModes (res9..res12); add 9 to 
   (uint8_t)<returned value> to get precision in bits
   (values 9 to 12).
*/
resModes DS18::getPrecision(uint8_t *addr) {
  uint8_t data[9];
  reset();
  select(addr);
  write(readSP);
  for (int i=0; i<9; i++) data[i] = read();
  reset();
  return ( (resModes)( (data[4] & 0x60) >> 5 ) );
};


/*  Writes the contents of Scratchpad memory into EEPROM
    so that settings (resolution and ID or TH/TL) will be
    restored after next power-up 
*/
void DS18::saveScratchpad(uint8_t *addr) {
    select(addr);
    write(copySP);                   // copy scratchpad to EEPROM
    return;
  };

/*  reads temperature of sensor at address "addr" with device type
    identified by dsID(addr[0]) and returns temperature as a 
    float number, in Celsius scale, along with raw data.
    If forceSample==true, cause this probe to sample temp
    If forceSample==false, just read the data (for concurrent conversion mode)
*/

float DS18::getTemperature(uint8_t *addr, uint8_t *data, boolean forceSample) {
  uint8_t myResMode;
  if (forceSample) {
    reset();                           // following datasheet specs start with reset
    select(addr);                      // select specific device
    write(convertT);                   // start conversion, NOT IN PARASITE MODE
                                     // Per OneWire docs, change that to 
                                     // write(converT,1) if DS18's are in parasite mode
                                     // but beware of possible device burnout
    // now wait for device ready or timeout
    for (int delayCount = 0; delayCount<1000 && read_bit()==0; delayCount++) delay(1);
  };
  reset();                           // reset again
  select(addr);                      // select that device again
  write(readSP);                     // Read Scratchpad to get temp reading
  for (int i=0; i<9; i++) data[i] = read();
  // End of datasheet prescription for reading temp.
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


/*  Causes all DS18 probes to sample concurrently, at whatever precision each one
    is set to use, and returns immediately WITHOUT WAITING FOR CONVERSION TO COMPLETE.
    Allows program to resume and allow conversions to continue concurrently.  
    Sets timer for use by "waitForTemps" to watch for timeout failure.
    USE "waitForTemps" TO WAIT FOR TEMP CONVERSIONS TO COMPLETE.
*/
void DS18::readAllTemps(void) {
  reset();
  write(skipROM);
  write(convertT);
  initTime = millis();
  return;
};

/*  Waits (maxWait, in msec) for DS18 probes concurrent sampling to complete.
    Delays until ALL probes report that they have completed conversions.
    Returns "true" if sampling completed in expected max time or
    returns "false" if sampling timed out.
*/
boolean DS18::waitForTemps(int maxWait) {
  int delayCount;
  for (delayCount = millis()-initTime; read_bit()==0 && delayCount<maxWait; delayCount++) delay(1);
  return delayCount<maxWait;
};                                     // end waitForTemps()

/*  Identifies the type of device if it is a DS18 we know about, otherwise
    marks it as unknown
*/
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


  /*  Sets the alarm upper & lower temps (data[2] and data[3], respectively).
      Each probe will register alarm status (respond to conditional search
      accordingly) if the temp *when conversion is triggered* is 
      greater than TH or less than TL.  Note that the probe has no
      memory to say if it has ever gone to alarm: it only reflects the
      status at the time of a commanded temperature conversion.

      NOTE WELL: This destroys the internal label within the probe;
      Must use 8-byte address to identify probe after this call
  */
void DS18::setAlarms(uint8_t *addr, int8_t TH, int8_t TL) {
  uint8_t data[9];
  reset();
  select(addr);
  write(readSP);
  for (int i=0; i<9; i++) data[i] = read();
  reset();
  data[2] = (uint8_t) TH;
  data[3] = (uint8_t) TL;
  reset();
  select(addr); 
  write(writeSP);                    // write 3 bytes to scratchpad  
  write_bytes( &data[2],3);
  write(copySP);                     // copy to EEPROM
  return;
};
