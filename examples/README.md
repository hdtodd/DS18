# DS18B20 Support Tools for Arduino
## A set of tools for the DS18B20 One-Wire Thermal Sensors

### Description

This package contains a set of tools to help characterize and manage the DS18B20 family of
thermal sensors via a One-Wire Protocol connection to the Arduino micro-controller system.
Tools include:

1.  A program to label the sensors through their internal 2-byte persistent storage

2.  A program to characterize the connected DS18B20 sensors.  (A large set of sensors can be
connected in parallel, concurrently, and characterized as a group.)  "Characterization" 
includes reporting the internal memory and reporting the temperature and conversion times for 9-, 
10-, 11-, and 12-bit conversion precisions for each of the connected sensors in turn.

3.  A program to report the temperatures for all connected DS18B20 sensors.

### Author
Written by HDTodd, September, 2015, Bozeman, MT.  

hdtodd@gmail.com
