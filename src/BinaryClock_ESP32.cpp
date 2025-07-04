#include <Arduino.h>

#include "BinaryClock.h"

using namespace BinaryClockShield;

// put function declarations here:

void setup()
   {
   // put your setup code here, to run once:
   BinaryClock::get_Instance().setup();
   }

void loop()
   {
   // put your main code here, to run repeatedly:
   BinaryClock::get_Instance().loop();
   }
