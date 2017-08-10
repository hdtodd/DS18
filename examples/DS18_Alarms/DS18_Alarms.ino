/* DS18_Alarms v2.0

   Demonstrates the use of the DS18 Class library 
   on an Arduino master to manage DS18-class thermal probes
   connected to the Arduino via OneWire communication system.
   This library extends the OneWire class of milesburton.com and pjrc.com.

   This program ets low-temp and high-temp threshholds on DS18 temperature 
   probes and then monitors those probe, reporting when the temperature
   exceeds the threshhold on any of them.

   Adapted from OneWire DS18S20, DS18B20, DS1822 Temperature Example
   http://www.pjrc.com/teensy/td_libs_OneWire.html
   http://milesburton.com/Dallas_Temperature_Control_Library

   Written by HDTodd, July, 2017, to use the DS18 library.
*/

#include <stdio.h>
#include "OneWire.h"
#include "DS18_Alarms.h"
#include "ChronodotI2C.h"

#define delayTime 60000		// delay time between sampling loop iterations, 60 sec

Chronodot RTC;
RTC_Millis Timer;               // Software clock
DS18 ds18(oneWirePin);		// Create the DS18 object.  Assumed to be
     				// on pin 5 in this example program; see .h

uint8_t addr[8], addrList[4][8], data[9];	// provide for up to 4 probes in alarm status
float celsius;
int8_t TL, TH; 			// Alarm temp limits, in Celsius
long markTime;			// used to record loop timings

void setup(void) {
  Serial.begin(9600);
  Timer.adjust(DateTime(__DATE__, __TIME__));  // set pseudo clock
  Serial.println("Testing program to demonstrate use of DS18 Class Library to read DS18 temperature sensors");
  Serial.println("\'Alarms' program: scans through all DS18\'s on the OneWire bus");
  Serial.println("  and allows the user to set low- and high-temp threshholds");
  Serial.println("  beyond which the probe will flag an alarm status.");
  Serial.println("Program then monitors attached probes for alarm conditions");
  Serial.println("  and prints messages when limits are exceeded.");
  Serial.println();
  Serial.println("Connect sensors to Arduino with 3-wire connection, NOT in parasitic mode");
  Serial.print(  "Connect data (yellow/white) to pin "); 
  Serial.print(  oneWirePin);
  Serial.println(" with 4K7 Ohm pullup to 3V3 VCC");
  Serial.println();
/* Confirm that there is at least one DS18-class device on the OneWire buss */
  if (!ds18.begin()) {
     Serial.println("[?Alarms]: No DS18 devices found on OneWire buss.");
     delay(60000);
     return;
     };
  ds18.reset();
  ds18.reset_search();
  delay(250);			// search resets require a 250 msec delay afterward

  Serial.println("Press <return> key when you're ready to get started");
  Serial.setTimeout(60000);
  while ( !Serial.available() ) {};
  Serial.flush();
/* Now set up alarm threshholds on each of the probes */
  Serial.println("Set the low- and high-temp threshholds for each probe.");
  Serial.println("Temps are integers -205F to +260F (-127 to +127 in Celsius).");
  while (ds18.search(addr)) {	// next device
    celsius = ds18.getTemperature(addr,data,true);
    Serial.print("For probe at address: 0x"); 
    for (int i=0; i<8; i++) Serial.write(HEX2(addr[i]));
    Serial.print("  Temp = "); Serial.print(celsius,2); Serial.print(" C");
    Serial.print("  with ");
    Serial.print( 9 + ((data[4] & 0x60)>>5) );
    Serial.println(" bit resolution");

    Serial.setTimeout(10000);
    Serial.print("\tLow threshhold (or -205 for none): ");
    TL=Serial.parseInt();
    TL = (TL<=-205) ? -205 : (int)FtoC( (float)TL );

    Serial.print("\tHigh threshhold (or 260 for none): ");
    TH = Serial.parseInt();
    TH = (TH>=260) ? +260 : (int)FtoC( (float)TH );

    Serial.print("\tSetting TL="); Serial.print(TL); 
    Serial.print("C and TH="); Serial.print(TH); Serial.println("C");
    ds18.setAlarms(addr,TH,TL);

    celsius = ds18.getTemperature(addr,data,true);
    Serial.print("\t==> Confirming that for probe 0x");
    for (int i=0; i<8; i++) Serial.write(HEX2(addr[i]));
    Serial.print(", TH = "); Serial.print((int8_t)data[2]);
    Serial.print(" & TL = "); Serial.println((int8_t)data[3]);
    Serial.print("  Data=0x");
    for (int j=0; j<8; j++) {Serial.print(HEX2(data[j])); Serial.write(' ');};
    Serial.println();
    };				// end while(search)
  ds18.reset();
  ds18.reset_search();
  delay(250);			// search resets require a 250 msec delay afterward

};				// end setup()

/* Trigger a temp conversion on all probes, scan for alarms, delay, and repeat
   According to data sheet, it *appears* that if you do a data transmission
   after the alarm_search, you have to do a reset and start over.  Not sure quite
   how practical that approach would be.  So here we just scan and tabulate
   the addresses of any probes in alarm status, then go back through each
   of those and log the time, TL/TH, and temps.  Other approaches might work,
   but this approach seemed safe and consistent with data sheet.
*/
void loop(void) {
  DateTime now;
  char dateString[20];

  ds18.reset();
  ds18.write(skipROM);		// talk to all OneWire devices
  ds18.write(convertT);		// tell them all to check temps
  markTime = -millis();		// prepare for timing
  for (int delayCount = 0; delayCount<1000 && ds18.read_bit()==0; delayCount++) delay(1);
  markTime += millis();	        // Note how long it actually took
  now = Timer.now();		// get current time for log
  sprintf(dateString, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(),
      now.hour(),now.minute(), now.second() );
  dateString[19] = '\0';
  Serial.print(dateString);     // Log event
  Serial.print(": Probe sampling took ");
  Serial.print(markTime);
  Serial.println(" msec");

  ds18.reset();
  ds18.reset_search();
  delay(250);
  int nextAlarm = 0;
  while ( ds18.search(addr,false) ) {	// get addr of next device in alarm
    for (int i=0; i<8; i++) addrList[nextAlarm][i] = addr[i]; 	    // add that addr to list
    nextAlarm++;
    };					// end while(ds18.alarm_search())

  for (int i=0; i<nextAlarm; i++) {
// If we got here, at least one DS18 is signalling alarm
// Go through the list and log details
    ds18.reset();
    markTime = -millis();				// mark start of conversion for timing
    celsius = ds18.getTemperature(addrList[i], data, true);    // returns when conversion complete
    markTime += millis();
    Serial.print("[%Alarm]: Address=0x"); 
    for (int j=0;j<8;j++) Serial.print(HEX2(addrList[i][j]));
    Serial.print("  Chip = "); Serial.print(listDS18s[ds18.idDS(addrList[i][0])].devName);
    Serial.print(" TL="); Serial.print((int8_t)data[3]); 
    Serial.print(" TH="); Serial.print((int8_t)data[2]);
    Serial.print("  Temp = ");
    Serial.print(celsius,1);
    Serial.print("C = ");
    Serial.print(CtoF(celsius),1);
    Serial.print("F");
    Serial.print("  Conv time = "); Serial.print(markTime); Serial.println(" msec");
    };		     	       	        // end for(i)

   delay(delayTime);			// wait a while before checking again
  };					// end loop()
