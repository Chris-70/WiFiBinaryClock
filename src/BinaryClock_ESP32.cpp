#include <Arduino.h>

#include "BinaryClock.h"

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

#define OLED_IIC_ADDR 0x3c

#define OLED_RESET 4
// Adafruit_SSD1306 display(OLED_RESET);

using namespace BinaryClockShield;
#define BINARYCLOCK  BinaryClock::get_Instance()
// put function declarations here:

BinaryClock& binclock = BinaryClock::get_Instance(); // Get the singleton instance of BinaryClock

void setup()
   {
   Serial.begin(115200); // Start the serial communication at 115200 baud rate

   // put your setup code here, to run once:
   // BinaryClock& clock = BinaryClock::get_Instance(); // Get the singleton instance of BinaryClock
   binclock.setup();
   binclock.set_Brightness(20);

   // // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
   // display.begin(SSD1306_SWITCHCAPVCC, OLED_IIC_ADDR);  // initialize with the I2C addr 0x3C (for the 128x32)
   }

void loop()
   {
   // put your main code here, to run repeatedly:
   // BINARYCLOCK.loop();
   binclock.loop();
   }
