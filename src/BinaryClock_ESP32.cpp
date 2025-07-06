#include <Arduino.h>

#include "BinaryClock.h"

using namespace BinaryClockShield;

// put function declarations here:

void setup()
   {
   Serial.begin(115200); // Start the serial communication at 115200 baud rate
//   BUILTIN_LED = 2; // Set the built-in LED pin to 2 for ESP32
   pinMode(LED_BUILTIN, OUTPUT); // Initialize the built-in LED pin as an output
   for (int i = 0; i < 5; i++)
      {
      digitalWrite(LED_BUILTIN, HIGH); // Turn on the built-in LED
      delay(250);
      digitalWrite(LED_BUILTIN, LOW); // Turn off the built-in LED
      delay(250);
      }

   // put your setup code here, to run once:
   BinaryClock::get_Instance().setup();
   }

void loop()
   {
   // put your main code here, to run repeatedly:
   BinaryClock::get_Instance().loop();
   }
