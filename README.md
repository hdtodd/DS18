# DS18: Support Library for OneWire-Connected DS18-Class Multiprobe Thermal Sensors
## Version 1.0

This DS18 Arduino library extends the Arduino OneWire library 
to create and support DS18 objects {DS18S20, DS18B20, and DS1822}.  See 
<http://milesburton.com/Dallas_Temperature_Control_Library> for information about
the OneWire library and for references to detailed device specifications.
This DS18 library also supports the OneWire2 library 
(<https://www.pjrc.com/teensy/td_libs_OneWire.html>) now recommended by Miles Burton. 

DS18 thermal sensors are connected to an Arduino with one data wire.  The OneWire system
allows multiple sensors to be connected in parallel, with each addressed uniquely by a 
64-bit address.  The OneWire library provides functions for resetting the devices, 
discovering the addresses of connected devices, sending commands to the devices,
retrieving information from the devices, etc.  Examples with that OneWire library show
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

Library functions include:

*  `DS18` Constructor
*  `bool begin()`: confirms the presence of at least one DS18 device on the OneWire buss
*  `setPrecision(addr, resMode)`: sets the resolution of the DS18 device at
address "addr" to 9, 10, 11, or 12 bits.
*  `float getTemperature(typeDS, addr, data)`: returns the temperature, in Celsius, 
of the DS18 device of type "typeDS" at address "addr".  Also returns the device
data string from which the temperature was determined, which might be useful for
debugging.

## Example Programs

The `./examples` directory includes programs to demonstrate the use of DS18 functions:

*  Labeler: Stores a two-character label in the EEPROM of each connected DS18 device
that can be subsequently used to identify each probe individually as temperatures are read.
*  Char: Characterizes each connected DS18 device on the OneWire buss by providing
address, triggering temperature conversion, reading the resulting data stream (including
two-character label), reading temperature at each of the four possible resolutions, 
and reporting the time for conversion at each resolution of each probe.
*  Temp: Demonstrates how to read the temperature of each probe.


## Author
Primary authors: Jim Studt, Miles Burton, Paul Stoffregen, and numerous other 
contributors.  This DS18 library was Written by David Todd, 
Bozeman MT/Williston VT, 2017-07-05, by extracting code
from the OneWire examples and consolidating into a DS18 class that extends the
OneWire class.  

