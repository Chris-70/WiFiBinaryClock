#include <Arduino.h>

#include "BinaryClock.h"

using namespace BinaryClockShield;

// put function declarations here:

void setup()
   {
   Serial.begin(115200); // Start the serial communication at 115200 baud rate

   // put your setup code here, to run once:
   BinaryClock::get_Instance().setup();
   BinaryClock::get_Instance().set_Brightness(20);
   }

void loop()
   {
   // put your main code here, to run repeatedly:
   BinaryClock::get_Instance().loop();
   }
