# DS18 Support Tools for Arduino v2.0
## A set of tools for the DS18 One-Wire Thermal Sensors

### Description

This package contains a set of tools to help characterize and manage the DS18 family of
thermal sensors via a One-Wire Protocol connection to the Arduino micro-controller system.  These programs use the DS18 library (DS18.h, DS18.cpp) that is an extension of the OneWire class library.  These programs are examples of the use of that DS18 library.  The examples use the following Arduino libraries: OneWire, I2C, ChronodotI2c, DS18.

Example tools include:

1.  A program to label the sensors through their internal 2-byte EEPROM storage so that
they retain their internal (software-readable) labels through power cycles.

2.  A program to characterize the connected DS18 sensors.   "Characterization" 
includes reporting the internal memory and reporting the temperature and conversion times 
for 9-, 10-, 11-, and 12-bit conversion precisions for each of the connected sensors in turn.  The example also demonstrates how to use the DS18 library to read all temperature probes in parallel.

3.  A program to report the temperatures for all connected DS18 sensors.

4.  A program to set lower/upper bounds for alarm status for all connected DS18 sensors
and then loop to monitor and to report any in alarm status.  This program also demonstrates
how to command concurrent temperature samplings for all connected probes.

### Repository
[https://github.com/hdtodd/DS18](https://github.com/hdtodd/DS18.git)

### Dependencies
1. [DS18 Class Library](https://github.com/hdtodd/DS18.git)
1. [Stoffregen/Burton/PJRC OneWire Class Library](https://github.com/PaulStoffregen/OneWire) [[Also check out Miles Burton's temperature control library] (https://github.com/milesburton/Arduino-Temperature-Control-Library)]
1. [ChronodotI2C for Alarm reporting](https://github.com/hdtodd/ChronodotI2C.git)
1. [Wayne Truchsess's I2C Library for ChronodotI2C/Alarm reporting](http://dsscircuits.com/images/code/I2C_Rev5.zip)


### Author
Originally written by David Todd, September, 2015; <hdtodd@gmail.com>.  Major additions and upgrades July/August, 2017.

### Change Log

*  v2.0, 2017-07-09: Revised to use DS18 objects, inheriting from OneWire; added alarms, concurrent conversions.

*  v1.0, 2015-09-??: Original version used OneWire library directly

### License
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

