# DS18 Support Tools for Arduino v2.0
## A set of tools for the DS18 One-Wire Thermal Sensors

### Description

This package contains a set of tools to help characterize and manage the DS18 family of
thermal sensors via a One-Wire Protocol connection to the Arduino micro-controller system.
These programs use the DS18 library (DS18.h, DS18.cpp) that is an extension of the OneWire
object library.  These programs represent "examples" of the use of that DS18 library.  The
examples use the following Arduino libraries: OneWire, I2C, ChronodotI2c, DS18.

Example tools include:

1.  A program to label the sensors through their internal 2-byte EEPROM storage so that
they retain their internal (software-readable) labels through power cycles.

2.  A program to characterize the connected DS18 sensors.  (A large set of sensors can be
connected in parallel, concurrently, and characterized as a group.)  "Characterization" 
includes reporting the internal memory and reporting the temperature and conversion times 
for 9-, 10-, 11-, and 12-bit conversion precisions for each of the connected sensors in turn.

3.  A program to report the temperatures for all connected DS18 sensors.

4.  A program to set lower/upper bounds for alarm status for all connected DS18 sensors
and then loop to monitor and to report any in alarm status.  This program also demonstrates
how to command concurrent temperature samplings for all connected probes.

### Author
Written by David Todd, September, 2015, Bozeman, MT: <hdtodd@gmail.com>

### Change Log

*  v2.0, 2017-07-09: Revised to use DS18 objects, inheriting from OneWire

*  v1.0, 2015-09-??: Original version used OneWire library directly

