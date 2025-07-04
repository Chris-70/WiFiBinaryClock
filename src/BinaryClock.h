// Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
// https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
//
// Binary Clock RTC 24H with Interrupt, Alarm and Buttons Example
// This example demonstrates complete Binary Clock with Time and Alarm settings
//
// *It is recommended that the first start should be carried out with the serial terminal, 
// for better knowing the setting options. 
//
// The buttons allows you to set the time and alarm - exact hour, minute, second/alarm status.
// Alarm causes melody to play.  
// How to use piezo with the tone() command to generate notes you can find here:
// http://www.arduino.cc/en/Tutorial/Tone
//
// A falling edge at the RTC INT/SQW output causes an interrupt, 
// which is uses for regular - 1 per second - reading time from RTC and 
// checking alarm status flag 'A2F'. Since we use RTC INT/SQW output for
// regular reading current time - square wave output SQW option, 
// global interrupt flag INTCN is set to 0, this disables the interrupts from both RTC alarms.
// Referring to the documentation: when the INTCN is set to logic 0, 
// the 'A2F' bit does not initiate an interrupt signal. By turning off the interrupts from the alarms, 
// we can use the interrupt flag 'A2IE' as an info flag whether the alarm has been activated or not. 
// Check RTC datasheet page 11-13 http://bit.ly/DS3231-RTC
//
// Hardware:
// Arduino Uno, Binary Clock Shield for Arduino
// Battery CR1216/CR1220 
// INT/SQW   connected to Arduino pin 3  INT1 ESP32_D1-R32 UNO 25
// PIEZO     connected to Arduino pin 11 PWM  ESP32_D1-R32 UNO 23
// S3 button connected to Arduino pin A0      ESP32_D1-R32 UNO  2
// S2 button connected to Arduino pin A1      ESP32_D1-R32 UNO  4
// S1 button connected to Arduino pin A2      ESP32_D1-R32 UNO 35
// LEDs      connected to Arduino pin A3      ESP32_D1-R32 UNO 32 (Requires ESP32UNO board modification to use pin 32))
// RTC SDA   connected to Arduino pin A4      ESP32_D1-R32 UNO 36
// RTC SCL   connected to Arduino pin A5      ESP32_D1-R32 UNO 39
//
//                        +------+       +------+       +------+       +------+       +------+
//                        |LED 16|---<---|LED 15|---<---|LED 14|---<---|LED 13|---<---|LED 12|--<-+
//                        +------+       +------+       +------+       +------+       +------+    |
//                                                                                                |
//    +--------------->-------------->-------------->-------------->-------------->---------------+
//    |
//    |    +------+       +------+       +------+       +------+       +------+       +------+
//    +----|LED 11|---<---|LED 10|---<---|LED 09|---<---|LED 08|---<---|LED 07|---<---|LED 06|--<-+
//         +------+       +------+       +------+       +------+       +------+       +------+    |
//                                                                                                |
//    +--------------->-------------->-------------->-------------->-------------->---------------+
//    |
//    |    +------+       +------+       +------+       +------+       +------+       +------+
//    +----|LED 05|---<---|LED 04|---<---|LED 03|---<---|LED 02|---<---|LED 01|---<---|LED 0 |--<-- DATA_PIN 
//         +------+       +------+       +------+       +------+       +------+       +------+
// 
// Note: (C.Guy 2025/07/03)
// =====
// This file has been adapted from the original Example; "11-BinaryClock-24H-RTCInterruptAlarmButtons.ino" file as published
// on the Binary Clock Shield for Arduino GitHub repository: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
// The original file was modified to be encapsulated in a class, BinaryClock, that encapsulates all the functionality
// of the Binary Clock Shield for Arduino. Modifications were made to support the ESP32 UNO platform and to allow
// greater flexibility by the user at runtime, such as the color selection and melodies used for the alarm.
// 
// The goal of using an ESP32 based UNO board was to allow the RTC to be connected to a NTP server over WiFi. The code
// for the WiFi connection is encapsulated in its own class, WiFiClock, which is not included in this file. It uses 
// WPS to connect to a WiFi network and stores the credentials in the ESP32 flash memory so future connections are
// made automatically without user intervention. The WiFi connection also allows the user to change the LED colors
// and melodies used for the alarm at runtime, without needing to recompile the code. 
//

#ifndef _BINARY_CLOCK_RTC_24_ALARM_BUTTONS_
#define _BINARY_CLOCK_RTC_24_ALARM_BUTTONS_

#include "pitches.h"            // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody
#include <FastLED.h>            // https://github.com/FastLED/FastLED
#include <DS3232RTC.h>          // https://github.com/JChristensen/DS3232RTC
#include <Streaming.h>          // https://github.com/janelia-arduino/Streaming                            

namespace BinaryClockShield
   {
   // This is a Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
   //
   #define ESP32UNO 
   #define SERIAL_SETUP  1      // If true - serial setup ON, if false serial setup OFF
   #define SERIAL_TIME   0      // If true - serial time  ON, if false serial time  OFF

   #ifdef ATMELUNO   // Standard Arduino UNO board definitions with the ATMEL chip
      // Arduino UNO ATMEL pin definitions
      #define INT           3         // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
      #define PIEZO         11        // The number of the Piezo pin
      #define LED_PIN       A3        // Data pin that LEDs data will be written out over

      #define S1  A2                  // Push buttons connected to the A0, A1, A2 Arduino pins
      #define S2  A1    
      #define S3  A0 
   #elif defined(ESP32_D1_R32) || defined(ESP32UNO) // ESP32 Wemos D1 R32 UNO board definitions
      // ESP32 UNO pin definitions
      #define INT          25        // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
      #define PIEZO        23        // The number of the Piezo pin
      #define LED_PIN      32        // Data pin that LEDs data will be written out. Requires ESP32UNO board modification to use pin 32
                                    // If you use ESP32UNO board, you need to modify the ESP32UNO board by removing the connector at pin 34
                                    // Solder a jumper wire from PIN 32 to the LED pin on the shield. 
                                    // ESP32UNO PIN 34 is Read-Only and cannot be used for output.

      #define S1  35                 // Push buttons connected to the A0, A1, A2 Arduino pins
      #define S2   4    
      #define S3   2 
   #else
      #error "Unsupported board. Please define the pin numbers for your board."
   #endif

   // The physical layout of the LEDs on the shield, one row each.
   #define NUM_HOUR_LEDS 5
   #define NUM_MINUTE_LEDS 6
   #define NUM_SECOND_LEDS 6
   #define NUM_LEDS (NUM_HOUR_LEDS + NUM_MINUTE_LEDS + NUM_SECOND_LEDS)

   #define LED_TYPE           WS2812B  // Datasheet: http://bit.ly/LED-WS2812B
   #define COLOR_ORDER         GRB     // For color ordering use this sketch: http://bit.ly/RGBCalibrate   

   #define DEFAULT_BRIGHTNESS   30     // The best tested LEDs brightness 20-60
   #define DEFAULT_ALARM_REPEAT  3     // How many times play the melody alarm

   class BinaryClock
      {
   public:
      void setup();
      void loop();
      void RTCinterrupt();
      void settingsMenu();
      void setCurrentModifiedValue();
      void checkCurrentModifiedValueFormat();
      void saveCurrentModifiedValue();
      void displayCurrentModifiedValue();
      void getAlarmTimeAndStatus();
      void setAlarmTimeAndStatus();
      void setNewTime();
      void getAndDisplayTime();
      void convertDecToBinaryAndDisplay(int bottomRow, int middleRow, int upperRow);
      void playAlarm();
      int  checkS1();
      int  checkS2();
      int  checkS3();
      void serialTime();
      void serialStartInfo();
      void serialSettings();
      void serialAlarmInfo();
      void serialCurrentModifiedValue();

      // Singleton pattern to ensure only one instance of BinaryClock
      static BinaryClock& get_Instance()
         {
         static BinaryClock instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }

   protected:
      BinaryClock(); // Constructor
      virtual ~BinaryClock(); // Destructor

      BinaryClock (const BinaryClock&) = delete;            // Disable copy constructor
      BinaryClock& operator=(const BinaryClock&) = delete;  // Disable assignment operator
      BinaryClock (BinaryClock&&) = delete;                 // Disable move constructor
      BinaryClock& operator=(BinaryClock&&) = delete;       // Disable move assignment operator

   public:         
      // These variables are intially set to the internal static melody and note durations arrays
      // They can be changed to use different melodies and note durations uploaded to the ESP32 flash memory.
      int  *melodyAlarm;         // Pointer to the melody array
      int   melodySize;          // Size of the melody array
      byte *noteDurations;       // Pointer to the note durations array
      int   noteDurationsSize;   // Size of the note durations array

      static CRGB OnColor [NUM_LEDS]     PROGMEM; // Colors for the LEDs when ON
      static CRGB OffColor[NUM_LEDS]     PROGMEM; // Colors for the LEDs when OFF

   protected:
      DS3232RTC RTC;                               // Create RTC object

      int alarmRepeatMax = DEFAULT_ALARM_REPEAT;   // Maximum alarm repeat count
      int alarmRepeatCount = 0;                    // Current alarm repeat count

      static CRGB leds[NUM_LEDS] PROGMEM;          // Array of LED colors to display the current time
      static bool binaryArray[NUM_LEDS] PROGMEM;   // Serial Debug: Array for binary representation of time

      // t variable for Arduino Time Library 
      time_t t;
      tmElements_t tm;
      // See the Arduino Time Library for details on the tmElements_t structure: 
      // http://playground.arduino.cc/Code/Time
      // https://github.com/PaulStoffregen/Time  

      // Note durations: 4 = quarter note, 8 = eighth note, etc.:
      // Some notes durations have been changed (1, 3, 6) to make them sound better
      static const byte NoteDurations[] PROGMEM;
      static const int  MelodyAlarm[] PROGMEM; // Melody for alarm
      static const int  MelodySize; // Size of the melody array
      static const int  NoteDurationsSize; // Size of the note durations array

   private:
      // The current reading from the input pins
      bool S1state = LOW;
      bool S2state = LOW;
      bool S3state = LOW;

      // The previous reading from the input pins
      bool lastreadS1 = LOW;
      bool lastreadS2 = LOW;
      bool lastreadS3 = LOW;

      int countButtonPressed;                // Counter for button pressed
      static volatile bool RTCinterruptWasCalled;   // Flag for RTC interrupt was called

      // The following variables are unsigned longs because the time, measured in
      // milliseconds, will quickly become a bigger number than can be stored in an int.
      unsigned long lastDebounceTime = 0;   // The last time the input pin was toggled
      unsigned long debounceDelay = 50;     // The debounce time. Increase if the output flickers

      // Variables that store the current settings option
      int settingsOption = 0;               // Time = 1, Alarm = 3  
      int settingsLevel = 0;                // Hours = 1, Minutes = 2, Seconds / On/Off Alarm = 3

      // Variables that store the current alarm time and status
      int hourAlarm = 0;
      int minuteAlarm = 0;
      int alarmStatus = 0;

      };
   }

#endif