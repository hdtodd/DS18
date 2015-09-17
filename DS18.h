// DS18.h
// DS18-class thermal sensor object definition
// 
// Created by HDTodd, September, 2015
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

typedef enum {readSP=0xBE, writeSP=0x4A, saveSP=0x48, sampleTemp=0x44}
  sensorCommands;
   
// readSP      read 9 bytes of ScratchPad memory over bus
// writeSP     write 3 bytes of ScratchPad to sensor SP 2-4
//               2-byte ID or TH/TL and 1 byte resolution setting
// saveSP      write 9 bytes of ScratchPad memory into SP EEPROM
// sampleTemp  start sampling of temp, converting to 2-byte digital
//               result left in SP bytes 0-1
typedef enum {res9=0b00, res10=0b01, res11=0b10, res12=0b11} resModes;
const uint16_t resMask[] = {~7, ~3, ~1, ~0};   // masks to truncate raw data
// Temp read conversion times, per Datasheet:
//    9-bit     93.75 ms
//   10-bit    187.5  ms
//   11-bit    375    ms
//   12-bit    750    ms

boolean firstPass=true;


// Search for the next OneWire device
// Returns 0 and fills in addr[] if found device
// Returns 1 and fills in addr[] if reset scan at beginning
//                               and found device
// Returns -1 if CRC error in reading device ROM
int DS18::nextAddr(byte *addr);


// Make sure the device at *addr is a DS18 device
//   the first ROM byte indicates which chip type
// Returns 0 if DS18B20 or 22
//         1 if DS18S20 (old chip)
//        -1 if not a DS18-type device
int DS18::verifyType(byte *addr);

//  set precision for temp conversion to 9, 10, 11, or 12 bits
//  addr[] contains OneWire addr of device; resMode is resolution desired
void DS18::setPrecision(byte *addr, resModes resMode);

//  Writes the contents of Scratchpad memory into EEPROM
//    so that settings (ID or TH/TL and resolution) will be
//    restored after next power-up 
void DS18::saveScratchpad(byte *addr);


//  reads temperature of sensor at address "addr"
//    known to be of type type_s
//  and returns value as a float number, in Celsius scale
float DS18::getTemperature(int type_s, byte *addr);

