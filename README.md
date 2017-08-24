# DS18: Arduino Class Library for OneWire-Connected DS18-Class Multiprobe Thermal Sensors
## Version 2.0

This DS18 Arduino library extends the Arduino OneWire library 
to create and support DS18 objects {DS18S20, DS18B20, and DS1822}.  See 
<http://milesburton.com/Dallas_Temperature_Control_Library> for information about
the OneWire library and for references to detailed device specifications.
This DS18 library uses the OneWire2 library 
(<https://www.pjrc.com/teensy/td_libs_OneWire.html>) now recommended by Miles Burton. Also review Miles Burton's DS18 Class library for a system with finer-grained control (<https://github.com/milesburton/Arduino-Temperature-Control-Library>).

DS18 thermal sensors are connected to an Arduino with one data wire.  The OneWire system
allows multiple sensors to be connected in parallel, with each addressed uniquely by a 
64-bit address.  The OneWire library provides functions for resetting the devices, 
discovering the addresses of connected devices, sending commands to the devices,
retrieving information from the devices, etc.   Examples with that OneWire library show
how to collect information from the DS18 devices as one of the demonstration programs.

This DS18 library extends that OneWire library as a DS18 class. 
It packages the DS18-specific functionality
as higher-level function calls to simplify use of the DS18 on Arduinos for common
applications.  The OneWire functions are available as 
inherited functionality, too, if more detailed control
of the devices is needed.

OneWire devices can be connected in powered mode or parasitic mode.  Parasitic mode
has just two wires: ground and data.  The devices charge themselves from the data line
when data is not being transmitted.  Parasitic mode requires that the master device
(the Arudino in this case) pay attention to the use of the buss so that it is not
held in a low (ground) state for extended periods of time.  This DS18 library 
presumes that the OneWire devices are connected with three-wire
connections -- power, ground, and data -- and does not manage the data buss directly. 

For more detailed information about individual devices and their operation, consult the appropriate datasheet from Maxim (e.g., for the DS18B20, see [https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf]()).

##  Functions Provided

Functions associated with the DS18 library include:

*  `DS18` Constructor
*  `bool begin()`: confirms the presence of at least one DS18 device on the OneWire buss
*  `setPrecision(addr, resMode)`: sets the resolution of the DS18 device at
address "addr" to 9, 10, 11, or 12 bits.
*  `float getTemperature(addr, data, forceSample)`: returns the temperature, in Celsius, 
of the DS18 device of type "typeDS" at address "addr".  Also returns the device
data string from which the temperature was determined, which might be useful for
debugging.  Can be used to force sampling for an individual probe (forceSample=true) or to retrieve the probe's temperature after concurrent sampling by all probes.
*  `resModes getPrecision(addr)`: returns the precision setting of the probe at address 'addr'.
*  `void saveScratchpad(addr)`: writes the scratchpad values of the probe at address 'addr' into its EEPROM for recovery after power cycling.
*   `void readAllTemps(void)`: causes all DS18 probes to begin sampling their temperatures concurrently.  
*   `boolean waitForTemps(maxWait)`: waits for concurrent sampling to complete or time out.
*   `typeDS idDS(devCode)`: identifies the type of the DS18 probe as one of a set of enumerated types.
*   `void setAlarms(addr, TL, TH)`: sets lower/upper temperature alarm parameters for the probe at address 'addr'.


## Example Programs

The `./examples` directory includes programs to demonstrate the use of DS18 functions:

*  Labeler: Stores a two-character label in the EEPROM of each connected DS18 device
that can be subsequently used to identify each probe individually as temperatures are read.
*  Char: Characterizes each connected DS18 device on the OneWire buss by providing
address, triggering temperature conversion, reading the resulting data stream (including
two-character label), reading temperature at each of the four possible resolutions, 
and reporting the time for conversion at each resolution of each probe.
*  Temp: Demonstrates how to read the temperature of each probe.
*  Alarms: Demonstrates how to set and monitor the temperatures of probes against lower/upper temperature min/max.

## Dependencies
1. [DS18 Class Library](https://github.com/hdtodd/DS18.git)
1. [Stoffregen/Burton/PJRC OneWire Class Library](https://github.com/PaulStoffregen/OneWire)  [[Also check out Miles Burton's Arduino temperature control library] (https://github.com/milesburton/Arduino-Temperature-Control-Library)]



## Author
Primary authors of the original OneWire Class library and DS18 routines: Jim Studt, Miles Burton, Paul Stoffregen, and numerous other 
contributors.  

This DS18 library was Written by David Todd, 
<hdtodd@gmail.com>, 2017-07-05, by extracting code
from the OneWire examples and consolidating into a DS18 class that extends the OneWire class.  


## License
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
