/* DS18_Char v3.0
   Demonstrate the use of the DS18 Class that extends 
   the OneWire Class of milesburton.com and pjrc.com.

   Characterize connected DS18-class temperature sensors attached
   to an Arduino master via OneWire communication system.
   Adapted from OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library

   Modified by HDTodd, August, 2017, to use the DS18 library.
*/

#include "OneWire.h"
#include "DS18_Char.h"

#define delayTime 10000		// delay time between sampling loop iterations, 10 sec
#define resetTime 250		// delay time req'd after search reset in msec
#define DEVMAX 10 		// max number of devices we're prepared to handle

DS18 ds18(oneWirePin);		// Create the DS18 object.  Assumed to be
     				// on pin 5 in this example program; see .h
long markTime, loopTime=0;	// used to record loop timings

dsInfo dsList[DEVMAX+1];	// list of DS devices found + 1 for looping safety
int devCount;			// number of DS devices found

void setup(void) {
  Serial.begin(9600,SERIAL_8N1);
  Serial.println("Testing program to demonstrate use of DS18 Class Library to read DS18 temperature sensors");
  Serial.println("Program cycles through all DS18B20\'s on the OneWire bus");
  Serial.println("  and times reading the temp off each one at 9-, 10-, 11-, and 12-bit");
  Serial.println("  resolutions -- 0.5, 0.25, 0.125, and 0.0625 degrees C");
  Serial.println();
  Serial.println("Connect sensors to Arduino with 3-wire connection, NOT in parasitic mode");
  Serial.print(  "Connect data (yellow/white) to pin "); 
  Serial.print(  oneWirePin);
  Serial.println("  with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println();
  if (!ds18.begin()) {
     Serial.println("[?DS18_Char]: No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  // We have OneWire devices, go enumerate them
  ds18.reset();
  ds18.reset_search();
  delay(resetTime);			// delay req'd after search reset

  // Scan for address of next device on OneWire
  for (devCount=0; (devCount<DEVMAX) && ds18.search(dsList[devCount].addr) ; devCount++) {
    // We have a device & space to store its info; verify it and record it or discount it
    // We'll discount if CRC isn't valid or if it's not a know DS18 type

    // First, confirm CRC of address to make sure we're getting valid data
    boolean validCRC=  (ds18.crc8(dsList[devCount].addr, 7) == dsList[devCount].addr[7]);

    // Confirm that it's a DS18 device and assign type
    dsList[devCount].type = ds18.idDS(dsList[devCount].addr[0]);

    // Ignore responses with invalid CRCs or from devices we don't know
    if ( !validCRC ||
         ( dsList[devCount].type == DSNull  ) ||
	 ( dsList[devCount].type == DSUnkwn ) ) 
      dsList[devCount--].alive = false;
    else
      dsList[devCount].alive = true;
  };

  // At this point, we've identified the DS18 devices, identified their types, and 
  //   recorded their information
  // We'll print that info in the loop that samples temps.  For now, just make sure
  //   we have at least SOME DS18 devices and that we didn't have too many
  // Then we're done with setup

  if (devCount<=0) {
    Serial.println("[?DS18_Char] No DS18-class devices found!");
    delay(60000);
    return;
  };

  if (devCount>=DEVMAX) {
    Serial.println("[%DS18_Char] Number of OneWire devices exceeds internal storage limit");
    Serial.print(  "             Only ");
    Serial.print(--devCount);
    Serial.print(" DS18 devices will be sampled.");
    delay(60000);
  };

  // clean up after device search 
  ds18.reset_search();		// reset search, 
  delay(resetTime);			// must wait at least 250 msec for reset search
  return;
};					// end setup()

void loop(void) {
  int i, dev, prec; 
  uint8_t resMode;
  uint8_t data[9];
  float celsius;
  
// SEQUENTIAL SECTION
// In this section, for each DS18 in turn, set the resolution to 9..12 bits and acquire and report the temp.
//

  for (i=0; i<115; i++) Serial.print('-'); Serial.println(); 
  Serial.println("NEXT SEQUENTIAL SCAN");
for (dev=0; dev<devCount; dev++) {
  Serial.print("Device Address =");		// report the address of the next device
  for(i = 0; i < 8; i++) {Serial.write(' '); Serial.print(HEX2(dsList[dev].addr[i])); };

  Serial.print("   Chip = "); Serial.println(listDS18s[dsList[dev].type].devName);

  for (resMode=0; resMode<=3; resMode++) {         // We'll cycle through all 4 resolutions
// Set precision for conversion
    ds18.reset();
    ds18.select(dsList[dev].addr);
    ds18.setPrecision(dsList[dev].addr,resMode);
    ds18.reset();
    markTime = -millis();			// mark start of conversion, to substract from finish time
    celsius = ds18.getTemperature(dsList[dev].addr, data, true);  // sample temp for this probe
    markTime += millis();

/* Report the probe data, the temp, and the timing for this probe
   We assume that internal non-volatile RAM on the probes has been
   previously set to be used as labels; TL/TH alarms not in use!
   Otherwise modify code below to write 8 probe address bytes as HEX2.
*/
    Serial.print("Sensor ID: \'"); Serial.print((char) data[2]); Serial.print((char) data[3]);
    Serial.print("\'\tData = ");
    for ( i = 0; i < 9; i++) { Serial.print(HEX2(data[i])); Serial.print(" "); };
    Serial.print(" CRC=");
    Serial.print(HEX2(OneWire::crc8(data, 8)));
    Serial.print("  Prec="); 
    if ( (prec=(9+(uint8_t)ds18.getPrecision(dsList[dev].addr))) < 10) Serial.write(' ');
    Serial.print(prec);
    Serial.print("  Conv time = "); 
    if (markTime<100) Serial.write(' '); Serial.print(markTime); 
    Serial.print("ms  Temperature = "); Serial.print(celsius); Serial.print("C = ");
    Serial.print(celsius * 1.8 + 32.0,1); Serial.println("F");
    };                                        // end for(resMode...)
  Serial.println();
};


// CONCURRENT SECTION
// In this section, we'll set the resolution to 9..12 bits, tell all DS18's to sample
//   at the same time, and then acquire the temp from each and report.

  for (i=0; i<149; i++) Serial.print('-'); Serial.println(); 
  Serial.println("NEXT CONCURRENT SCAN");
  for (resMode=0; resMode<=3; resMode++) {         // We'll cycle through all 4 resolutions
    for (dev=0; dev<devCount; dev++) {	 	   // For each DS18 device we know about ...
      ds18.reset();
      ds18.select(dsList[dev].addr);
      ds18.setPrecision(dsList[dev].addr,resMode);	// Set precision for conversion
    };

    // Now tell them all to sample temp
    markTime = -millis();				// mark start of conversion
    ds18.readAllTemps();				// do sampling
    ds18.waitForTemps(convDelay[(int)resMode]);
    markTime += millis();				// finish time minus start time

/* Finally, for each device, get its temp and report
   Report the probe data, the temp, and the timing for this probe
   We assume that internal non-volatile RAM on the probes has been
   previously set to be used as labels; TL/TH alarms not in use!
*/
    for (dev=0; dev<devCount; dev++) {
      Serial.print("Sensor ID: \'"); Serial.print((char) data[2]); Serial.print((char) data[3]);
      Serial.print(" Address =");		// report the address of the device
      for (i = 0; i < 8; i++) {Serial.write(' '); Serial.print(HEX2(dsList[dev].addr[i])); };
      Serial.print(" Chip = "); Serial.print(listDS18s[dsList[dev].type].devName);  // chip type
      celsius = ds18.getTemperature(dsList[dev].addr, data, false);  // sample temp for this probe
      Serial.print(" Data = ");			      // report the data string
      for ( i = 0; i < 9; i++) { Serial.print(HEX2(data[i])); Serial.print(" "); };
      Serial.print(" CRC=");   	 		// report the computed CRC for verification
      Serial.print(HEX2(OneWire::crc8(data, 8)));
      Serial.print("  Prec=");
      if ( (prec=(9+(uint8_t)ds18.getPrecision(dsList[dev].addr))) < 10) Serial.write(' ');
      Serial.print(prec);
      Serial.print(" bits  Temperature = "); Serial.print(celsius); Serial.print("C = ");
      Serial.print(celsius * 1.8 + 32.0,1); Serial.println("F");
    };						// end for(dev...)
  Serial.print("  Conv time = "); 
  if (markTime<100) Serial.write(' '); Serial.print(markTime); 
  Serial.println("msec");
  };						// end for(resMode...)
  delay(delayTime);			// wait before scanning again
};						// end loop

