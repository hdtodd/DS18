// Header file for DS18B internal labeling program, DS18_Labeler.h

#include <inttypes.h>
#include <Arduino.h>
#include "DS18.h"

#define oneWirePin 5          // We'll use Uno pin 5 for OneWire connections
                              // Connect the DS18 data wire to Uno pin 5
                              //  with 4K7 Ohm pullup to VCC 3V3
                              // And connect DS18 VCC and GND pins to Uno
                              // We use powered rather than parasitic mode



