/* DS18.h
   DS18-class thermal sensor object definition

   Created by HDTodd, 2017-July
   OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library
*/

#ifndef DS18_h
#define DS18_h

#include <inttypes.h>
#include "Arduino.h"
#include "OneWire.h"

//  Devices we know about:
struct devTuple {uint8_t devCode; const char* devName; };
const devTuple listDS18s[4] = { {0x10, "DS18S20" },
  			        {0x22, "DS1822"  },
			        {0x28, "DS18B20" },
			        {0xFF, "DSUnkwn"} };
	 	      	  
typedef enum {DS18S20=0x10, DS1822=0x22, DS18B20=0x28, DSUnkwn=0xFF} typeDS;

//  See DS18B20.pdf datasheet for command codes
typedef enum {searchROM=0xF0, readROM=0x33, matchROM=0x55, skipROM=0xCC,
              alarmSearch=0xEC, convertT=0x44, writeSP=0x4A, readSP=0xBE, 
              copySP=0x48, recallEEPROM=0xB8, readPower=0xB4} sensorCommands;
/* readSP      read 9 bytes of ScratchPad memory over bus
   writeSP     write 3 user bytes of ScratchPad to sensor SP 2-4
                 2-byte ID or TH/TL and 1 byte configuration setting
   copySP      write 9 bytes of ScratchPad memory into SP EEPROM
   convertT    start sampling of temp, converting to 2-byte digital
                 result left in SP bytes 0-1
*/

typedef enum resModes {res9=0b00, res10=0b01, res11=0b10, res12=0b11} resModes;
const int resMask[4] = {~7, ~3, ~1, ~0};   // masks to truncate raw data

class DS18: public OneWire {

private:
    uint8_t _pinnumber;
    //    const int convDelay[4] = {94,188,375,750};
//  Temp read conversion times, per Datasheet:
//      9-bit     93.75 ms
//     10-bit    187.5  ms
//     11-bit    375    ms
//     12-bit    750    ms

public:

  /* constructor -- needs to know pin # to pass to OneWire()
  */
  DS18(uint8_t);

  /* Discover any DS18-class devices on the OneWire buss.  Library
     recognizes {DS18S20, DS18B20, DS1822} and knows how to set 
     resolution and read temps for those devices.
     Returns (bool)true if there is at least one DS18-class device on 
     the buss or (bool)false if there isn't.
  */
  bool begin(void);

  /*  set precision for temp conversion to 9, 10, 11, or 12 bits
      addr[] contains OneWire addr of device; resMode is resolution desired
  */
  void setPrecision(uint8_t *addr, uint8_t resMode);

  /*  Writes the contents of Scratchpad memory into EEPROM
        so that settings (ID or TH/TL and resolution) will be
        restored after next power-up 
  */
  void saveScratchpad(uint8_t *addr);

  /*  reads temperature of sensor at address "addr" with device type
      identified by dsID(addr[0]) and returns value as a 
      float number, in Celsius scale, along with raw data
  */
  float getTemperature(uint8_t *addr, uint8_t *data);

  /*  Uses the device code (addr[0]) to identify the type of DS device
      Return the DS18-class device-type id
  */
  typeDS idDS(uint8_t devCode);

};                                   // end class

#endif                               // #end ifndef DS18
