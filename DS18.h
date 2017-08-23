/* DS18.h
   DS18-class thermal sensor object definition

   Created by HDTodd, 2017-August
   OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library
*/

#ifndef DS18_h
#define DS18_h

#include <inttypes.h>
#include "Arduino.h"
#include "OneWire.h"

//  Devices we know about.  Keep Null first and Unkwn last if adding other devices,
//    else fix up DS18::idDS lookup procedure.
typedef enum {DSNull, DS18S20, DS1822, DS18B20, DSUnkwn} typeDS;
struct devTuple {uint8_t devCode; typeDS devID; const char* devName; };
// Device codes, enumerated type names, and labels for known DS18 devices
const devTuple listDS18s[5] = { {0x00, DSNull,  "DSNoCod" },
				{0x10, DS18S20, "DS18S20" },
  			        {0x22, DS1822,  "DS1822"  },
			        {0x28, DS18B20, "DS18B20" },
			        {0xFF, DSUnkwn, "DSUnkwn" } };

//  Device info structure: address, type, and (boolean) whether it still responds
typedef struct dsInfo { uint8_t addr[8]; typeDS type; boolean alive; } dsInfo;

//  See DS18B20.pdf datasheet for command codes
typedef enum {searchROM=0xF0, readROM=0x33, matchROM=0x55, skipROM=0xCC,
              alarmSearch=0xEC, convertT=0x44, writeSP=0x4E, readSP=0xBE, 
              copySP=0x48, recallEEPROM=0xB8, readPower=0xB4} sensorCommands;
/* readSP      read 9 bytes of ScratchPad memory over bus
   writeSP     write 3 user bytes of ScratchPad to sensor SP 2-4
                 2-byte ID or TH/TL and 1 byte configuration setting
   copySP      write 9 bytes of ScratchPad memory into SP EEPROM
   convertT    start sampling of temp, converting to 2-byte digital
                 result left in SP bytes 0-1
*/

static int convDelay[4] = {94,188,375,750};

typedef enum resModes {res9=0b00, res10=0b01, res11=0b10, res12=0b11} resModes;
const int resMask[4] = {~7, ~3, ~1, ~0};   // masks to truncate raw data
char *HEX2(uint8_t x);		// used to print hex numbers with 2 digits always
float FtoC(float F);
float CtoF(float C);

class DS18: public OneWire {

private:
    uint8_t _pinnumber;

//  Temp read conversion times, per Datasheet:
//      9-bit     93.75 ms
//     10-bit    187.5  ms
//     11-bit    375    ms
//     12-bit    750    ms

public:

  /* constructor -- needs to know pin # to pass to OneWire()
  */
  DS18(uint8_t pinnumber);

  /* Discover any DS18-class devices on the OneWire buss.  Library
     recognizes {DS18S20, DS18B20, DS1822} and knows how to set 
     resolution and read temps for those devices.
     Returns (boolean)true if there is at least one DS18-class device on 
     the buss or (boolean)false if there isn't.
  */
  boolean begin(void);

  /*  set precision for temp conversion to 9, 10, 11, or 12 bits
      addr[] contains OneWire addr of device; resMode is resolution desired
  */
  void setPrecision(uint8_t *addr, uint8_t resMode);

/* Returns the precision of conversion being used by the
   DS18 device at "addr".  Precision is returned as the
   enumerated type identifier resModes (res9..res12); add 9 to 
   (uint8_t)<returned value> to get precision in bits
   (values 9 to 12).
*/
  resModes getPrecision(uint8_t *addr);

/*  Writes the contents of Scratchpad memory into EEPROM
    so that settings (resolution and ID or TH/TL) will be
    restored after next power-up 
*/
  void saveScratchpad(uint8_t *addr);

/*  reads temperature of sensor at address "addr" with device type
    identified by dsID(addr[0]) and returns value as a 
    float number, in Celsius scale, along with raw data.
    If forceSample==true, cause this probe to sample temp
    If forceSample==false, just read the data (for concurrent sampling mode)
*/
  float getTemperature(uint8_t *addr, uint8_t *data, boolean forceSample);

/*  Causes all DS18 probes to sample concurrently, at whatever precision each one
    is set to use, and returns immediately WITHOUT WAITING FOR CONVERSION TO COMPLETE.
    Allows program to resume and allow conversions to continue concurrently.  
    Sets timer for use by "waitForTemps" to watch for timeout failure.
    USE "waitForTemps" TO WAIT FOR TEMP CONVERSIONS TO COMPLETE.
*/
  void readAllTemps(void);

/*  Waits (maxWait, in msec) for DS18 probes concurrent sampling to complete.
    Delays until ALL probes report that they have completed conversions.
    Returns "true" if sampling completed in expected max time or
    returns "false" if sampling timed out.
*/
  boolean waitForTemps(int maxWait);

/*  Uses the device code (addr[0]) to identify the type of DS device
    Return the DS18-class device-type id
*/
  typeDS idDS(uint8_t devCode);

/*  Sets the upper & lower temps (data[2] and data[3], respectively).
    Each probe will register alarm status (respond to conditional search
    accordingly) the the temp *when conversion is triggered* is 
    greater than TH or less than TL.  Note that the probe has no
    memory to say if it has ever gone to alarm: it only reflects the
    status at the time of a commanded temperature conversion.

    NOTE WELL: This destroys the internal label within the probe;
    Must use 8-byte address to identify probe after this call
*/
void setAlarms(uint8_t *addr, int8_t TH, int8_t TL);

};                                   // end class

#endif                               // #end ifndef DS18
