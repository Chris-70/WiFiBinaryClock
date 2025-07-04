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
// LEDs      connected to Arduino pin A3      ESP32_D1-R32 UNO 34
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

#include "BinaryClock.h"

// Include libraries
#include <FastLED.h>            // https://github.com/FastLED/FastLED
#include <DS3232RTC.h>          // https://github.com/JChristensen/DS3232RTC
#include <Streaming.h>          // https://github.com/janelia-arduino/Streaming                            
#include "pitches.h"            // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   // This is a Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
   //

   // Notes in the melody: (70)
   const int BinaryClock::MelodyAlarm[] PROGMEM =
         {
         NOTE_A4,  NOTE_A4,  NOTE_A4,  NOTE_F4,  NOTE_C5,  NOTE_A4,  NOTE_F4,  NOTE_C5,
         NOTE_A4,  NOTE_E5,  NOTE_E5,  NOTE_E5,  NOTE_F5,  NOTE_C5,  NOTE_GS4, NOTE_F4,
         NOTE_C5,  NOTE_A4,  NOTE_A5,  NOTE_A4,  NOTE_A4,  NOTE_A5,  NOTE_GS5, NOTE_G5,
         NOTE_FS5, NOTE_F5,  NOTE_FS5, 0,        NOTE_AS4, NOTE_DS5, NOTE_D5,  NOTE_CS5,
         NOTE_C5,  NOTE_B4,  NOTE_C5,  0,        NOTE_F4,  NOTE_GS4, NOTE_F4,  NOTE_A4,
         NOTE_C5,  NOTE_A4,  NOTE_C5,  NOTE_E5,  NOTE_A5,  NOTE_A4,  NOTE_A4,  NOTE_A5,
         NOTE_GS5, NOTE_G5,  NOTE_FS5, NOTE_F5,  NOTE_FS5, 0,        NOTE_AS4, NOTE_DS5,
         NOTE_D5,  NOTE_CS5, NOTE_C5,  NOTE_B4,  NOTE_C5,  0,        NOTE_F4,  NOTE_GS4,
         NOTE_F4,  NOTE_C5,  NOTE_A4,  NOTE_F4,  NOTE_C5,  NOTE_A4,
         };

   // Note durations: 4 = quarter note, 8 = eighth note, etc.:
   // Some notes durations have been changed (1, 3, 6) to make them sound better
   const byte BinaryClock::NoteDurations[] PROGMEM =
         {
         2, 2, 2, 3, 6, 2, 3, 6,
         1, 2, 2, 2, 3, 6, 2, 3,
         6, 1, 2, 3, 6, 2, 4, 4,
         8, 8, 4, 3, 4, 2, 4, 4,
         8, 8, 4, 3, 6, 2, 3, 6,
         2, 3, 6, 1, 2, 3, 8, 2,
         4, 4, 8, 8, 4, 4, 4, 2,
         4, 4, 8, 8, 4, 4, 4, 2,
         3, 8, 2, 3, 8, 1,
         };

   const int BinaryClock::MelodySize = sizeof(BinaryClock::MelodyAlarm) / sizeof(int);           // Size of the melody array (e.g. 70 notes)
   const int BinaryClock::NoteDurationsSize = sizeof(BinaryClock::NoteDurations) / sizeof(byte); // Size of the note durations array (70)
   
   // Default: Colors for the LEDs when ON, Seconds, Minutes and Hours
   CRGB BinaryClock::OnColor[NUM_LEDS] = 
         {
         CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,    // Seconds
         CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,  // Minutes
         CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Blue                 // Hours
         };
   // Default: Colors for the hours LEDs when OFF (Unually Black or No Power (i.e. OFF), Seconds, Minutes and Hours)
   CRGB BinaryClock::OffColor[NUM_LEDS] = 
         {
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Seconds
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Minutes
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black                // Hours
         };

   CRGB BinaryClock::leds[NUM_LEDS] = {};        // Array of LED colors to display the current time
   bool BinaryClock::binaryArray[NUM_LEDS] = {}; // Serial Debug: Array for binary representation of time
   
   // Flag to indicate the RTC interrupt was called (Class Global)
   volatile bool BinaryClock::RTCinterruptWasCalled;

   //################################################################################//
   // SETUP
   //################################################################################//
   void BinaryClock::setup()
      {
      #if SERIAL_SETUP || SERIAL_TIME
      Serial.begin(115200);
      Serial.println("Binary Clock Shield for Arduino\n");
      #endif

      RTC.begin();
      // Important power-up safety delay
      delay(3000);

      // Limit my draw to 450mA at 5V of power draw
      FastLED.setMaxPowerInVoltsAndMilliamps(5, 450);

      // Initialize LEDs
      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
      FastLED.setBrightness(DEFAULT_BRIGHTNESS);

      // Initialize the buttons pins as an input
      pinMode(S1, INPUT);
      pinMode(S2, INPUT);
      pinMode(S3, INPUT);

      getAlarmTimeAndStatus();
      serialStartInfo();

      // Configure an interrupt on the falling edge from the RTC INT/SQW output
      pinMode(INT, INPUT_PULLUP);
      // Attach the interrupt to the static RTCinterrupt function
      //attachInterrupt(digitalPinToInterrupt(INT), BinaryClock::RTCinterrupt, FALLING);
      // Attach the interrupt to the member RTCinterrupt function using a lambda
      attachInterrupt(
            digitalPinToInterrupt(INT),
            []() { BinaryClock::get_Instance().RTCinterrupt(); },
            FALLING);

      // Clear the alarm status flag 'A2F'
      RTC.alarm(DS3232RTC::ALARM_2);

      // Enable 1 Hz square wave RTC SQW output
      RTC.squareWave(DS3232RTC::SQWAVE_1_HZ);
      }

   void BinaryClock::RTCinterrupt()
      {
      BinaryClock::RTCinterruptWasCalled = true;
      }

   //################################################################################//
   // MAIN LOOP
   //################################################################################//
   void BinaryClock::loop()
      {
      settingsMenu();

      if (BinaryClock::RTCinterruptWasCalled && (settingsOption == 0))   // Display time but not during settings
         {
         BinaryClock::RTCinterruptWasCalled = false;       // Clear the interrupt flag
         getAndDisplayTime();                              // Get time from RTC, convert to binary format and display on LEDs
         serialTime();                                // Use serial monitor for showing current time 

         if ((RTC.alarm(DS3232RTC::ALARM_2)) & (alarmStatus == 2))
            {
            #if SERIAL_TIME
            Serial << "   ALARM!\n";
            #endif  
            playAlarm();
            }
         }
      }

   BinaryClock::BinaryClock() :
         countButtonPressed(0),                     // Initialize the button counter
         S1state(LOW),                              // Initialize the S1 button state
         S2state(LOW),                              // Initialize the S2 button state
         S3state(LOW),                              // Initialize the S3 button state
         lastreadS1(LOW),                           // Initialize the last read S1 button state
         lastreadS2(LOW),                           // Initialize the last read S2 button state
         lastreadS3(LOW),                           // Initialize the last read S3 button state
         settingsOption(0),                          // Initialize the settings option
         settingsLevel(0)                            // Initialize the settings level
      {
      melodyAlarm = (int*)MelodyAlarm;          // Assign the melody array to the pointer
      melodySize = MelodySize;                  // Assign the size of the melody array
      noteDurations = (byte*)NoteDurations;     // Assign the note durations array to the pointer
      noteDurationsSize = NoteDurationsSize;    // Assign the size of the note
      assert(melodySize == noteDurationsSize);  // Ensure the melody and note durations arrays are the same size
      }
   
   BinaryClock::~BinaryClock()
      {
      // Destructor
      // No dynamic memory allocation, nothing to clean up
      }

   //################################################################################//
   // SETTINGS
   //################################################################################//
   //
   //                       +-------------------------------+
   //                       |           SETTINGS            |
   //           +-----------+-------------------------------+
   //           |  BUTTONS  |    S3   |     S2    |   S1    |
   // +---------+-----------+---------+-----------+---------+
   // |         |           |         |   ALARM   |         |
   // |         |     0     |  ALARM  |   MELODY  |  TIME   |
   // |   S     |           |         |   STOP    |         |
   // +   E L   +-----------+---------+-----------+---------+
   // |   T E   | ROW H = 1 |    +    |   SAVE    |    -    |
   // |   T V   |           |         | LEVEL = 2 |         |
   // +   I E   +-----------+---------+-----------+---------+
   // |   N L   | ROW M = 2 |    +    |   SAVE    |    -    |
   // |   G     |           |         | LEVEL = 3 |         |
   // +   S     +-----------+---------+-----------+---------+
   // |         | ROW S = 3 |    +    |   SAVE    |    -    |
   // |         |           |         | LEVEL = 0 |         |
   // +---------+-----------+---------+-----------+---------+
   //
   //                       +-------------------------------+
   //                       |        SETTINGS OPTION        |
   //                       +---------------+---------------+
   //                       |   ALARM = 3   |   TIME = 1    |
   // +---------------------+---------------+---------------+
   // |   S     | ROW H = 1 |     3/1       |     1/1       |
   // |   E L   |           |     HOUR      |     HOUR      |
   // +   T E   +-----------+---------------+---------------+
   // |   T V   | ROW M = 2 |     3/2       |     1/2       |
   // |   I E   |           |    MINUTE     |    MINUTE     |
   // +   N L   +-----------+---------------+---------------+
   // |   G     | ROW S = 3 |     3/3       |     1/3       |
   // |   S     |           | ALARM STATUS  |    SECOND     |
   // +---------+-----------+---------------+---------------+
   //
   // The core of the settings menu
   void BinaryClock::settingsMenu()
      {
      // Main menu
      if ((settingsOption == 0) & (settingsLevel == 0))
         {
         // Time settings
         if (checkS1() == HIGH)
            {
            t = RTC.get();                          // Read time from RTC 
            settingsOption = 1;                     // Set time option settings
            settingsLevel = 1;                      // Set hour level settings
            setCurrentModifiedValue();              // Assign hours for modify +/- 
            serialSettings();                  // Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current hour on LEDs
            }

         // Alarm settings
         if (checkS3() == HIGH)
            {
            getAlarmTimeAndStatus();                // Read alarm time and status from RTC
            settingsOption = 3;                     // Set Alarm time option settings
            settingsLevel = 1;                      // Set hour level settings
            setCurrentModifiedValue();              // Assign hours for modify +/-
            serialSettings();                  // Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current alarm hour on LEDs 
            }
         }

      // Any settings option level except main menu
      if (settingsLevel != 0)
         {
         // Decrement
         if (checkS1() == HIGH)
            {
            countButtonPressed--;                   // Decrement current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();      // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();          // Display current modified value on LEDs 
            serialCurrentModifiedValue();      // Use serial monitor for showing settings
            }

         // Increment
         if (checkS3() == HIGH)
            {
            countButtonPressed++;                   // Increment current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();      // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();          // Display current modified value on LEDs  
            serialCurrentModifiedValue();      // Use serial monitor for showing settings 
            }

         // Save
         if (checkS2() == HIGH)
            {
            saveCurrentModifiedValue();             // Save current value e.g. hour, minute, second, alarm status     
            settingsLevel++;                        // Go to next settings level - hour => minute => second / alarm status 

            if (settingsLevel > 3)                  // If escape from settings then return to main menu
               {
               if (settingsOption == 1)            // If you were in the process of setting the time:   
                  {
                  setNewTime();                  // Save time to the RTC
                  }

               if (settingsOption == 3)            // If you were in the process of setting the alarm: 
                  {
                  setAlarmTimeAndStatus();        // Save time and alarm status to the RTC    
                  }

               serialAlarmInfo();             // Show the time and alarm status info when you exit to the main menu
               settingsLevel = 0;                  // Escape to main menu  
               settingsOption = 0;                 // Escape to main menu
               }
            else                                    // If you do not go to the main menu yet
               {
               checkCurrentModifiedValueFormat();  // Check if the value has exceeded the range e.g minute = 60 and correct                  
               setCurrentModifiedValue();          // Assign next variable for modify +/- hour => minute => second / alarm status
               displayCurrentModifiedValue();      // Display current modified value on LEDs                
               serialSettings();              // Use serial monitor for showing settings
               }
            }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, assign to the 
   // countButtonPressed variable to be able modify the value
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::setCurrentModifiedValue()
      {
      // Assign current time value stored in time_t t variable for modification
      if (settingsOption == 1)
         {
         if (settingsLevel == 1)  countButtonPressed = hour(t);
         if (settingsLevel == 2)  countButtonPressed = minute(t);
         if (settingsLevel == 3)  countButtonPressed = second(t);
         }

      // Alarm time and alarm status 
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)  countButtonPressed = hourAlarm;
         if (settingsLevel == 2)  countButtonPressed = minuteAlarm;
         if (settingsLevel == 3)  countButtonPressed = alarmStatus;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Check current modified value format of the countButtonPressed variable
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::checkCurrentModifiedValueFormat()
      {
      // Hours 0-23       
      if (settingsLevel == 1)
         {
         if (countButtonPressed < 0) countButtonPressed = 23;
         if (countButtonPressed > 23) countButtonPressed = 0;
         }

      // Minutes 0-59
      if (settingsLevel == 2)
         {
         if (countButtonPressed < 0) countButtonPressed = 59;
         if (countButtonPressed > 59) countButtonPressed = 0;
         }

      // Seconds & Alarm status
      if (settingsLevel == 3)
         {
         // Seconds 0-59
         if (settingsOption == 1)
            {
            if (countButtonPressed < 0) countButtonPressed = 59;
            if (countButtonPressed > 59) countButtonPressed = 0;
            }

         // Alarm status 1/2
         if (settingsOption == 3)
            {
            if (countButtonPressed < 1) countButtonPressed = 2;
            if (countButtonPressed > 2) countButtonPressed = 1;
            }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, save the current modified value
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::saveCurrentModifiedValue()
      {
      // Save current value in the tmElements_t tm structure
      if (settingsOption == 1)
         {
         if (settingsLevel == 1)  tm.Hour = countButtonPressed;
         if (settingsLevel == 2)  tm.Minute = countButtonPressed;
         if (settingsLevel == 3)  tm.Second = countButtonPressed;
         }

      // Alarm time and alarm status
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)  hourAlarm = countButtonPressed;
         if (settingsLevel == 2)  minuteAlarm = countButtonPressed;
         if (settingsLevel == 3)  alarmStatus = countButtonPressed;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Display on LEDs only currently modified value
   // convertDecToBinaryAndDisplay(hours row, miinutes row, seconds row)
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::displayCurrentModifiedValue()
      {
      if (settingsLevel == 1) { convertDecToBinaryAndDisplay(countButtonPressed, 0, 0); } // Hours
      if (settingsLevel == 2) { convertDecToBinaryAndDisplay(0, countButtonPressed, 0); } // Minutes
      if (settingsLevel == 3) { convertDecToBinaryAndDisplay(0, 0, countButtonPressed); } // Seconds
      }

   //################################################################################//
   // ALARM HANDLING
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Get alarm time and convert it from BCD to DEC format
   // Get alarm status active/inactive
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::getAlarmTimeAndStatus()
      {
      // Alarm Time HH:MM
      // 0x0B - alarm2 minute register address - check DS3231 datasheet
      // minutes using 7 bits of that register 
      minuteAlarm = RTC.readRTC(0x0B);
      minuteAlarm = ((minuteAlarm & 0b01110000) >> 4) * 10 + (minuteAlarm & 0b00001111);

      // 0x0C - alarm2 hour register address - check DS3231 datasheet 
      // hours using 6 bits of that register 
      hourAlarm = RTC.readRTC(0x0C);
      hourAlarm = ((hourAlarm & 0b00110000) >> 4) * 10 + (hourAlarm & 0b00001111);

      // Alarm Status active/inactive
      // Check bit 2 of the control register - A2IE
      // if 1 - alarm active, if 0 - alarm inactive 
      byte currentAlarmStatus = RTC.readRTC(0x0E) && 0b00000010;

      // alarmStatus: 1 - alarm inactive, 2 - alarm active   
      // it is the simplest way to display alarm status on LEDs (bottom S-row)
      // because with 0 and 1 - for 0 no LEDs lit, and this could be misleading
      if (currentAlarmStatus == 0) alarmStatus = 1;
      else if (currentAlarmStatus == 1) alarmStatus = 2;
      }
   ////////////////////////////////////////////////////////////////////////////////////
   // Set alarm time and status
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::setAlarmTimeAndStatus()
      {
      // Set alarm time  
      RTC.setAlarm(DS3232RTC::ALM2_MATCH_HOURS, 0, minuteAlarm, hourAlarm, 0);

      // Read RTC control register   
      byte controlRegister = RTC.readRTC(0x0E);

      // If the current alarm status is active, set bit 2 of the control register - A2IE
      // If inactive, clear bit 2 of the control register - A2IE  
      if (alarmStatus == 2) controlRegister = controlRegister | 0b00000010;
      else controlRegister = controlRegister & 0b11111101;

      // Update the control register
      RTC.writeRTC(0x0E, controlRegister);
      }

   //################################################################################//
   // RTC TIME & BINARY FORMAT
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Write time to RTC
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::setNewTime()
      {
      // Save the time stored in tmElements_t tm structure
      RTC.write(tm);
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Get time from RTC and convert to BIN format
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::getAndDisplayTime()
      {
      // Read time from RTC  
      t = RTC.get();
      // Convert time to binary format and display     
      convertDecToBinaryAndDisplay(hour(t), minute(t), second(t));
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Convert values from DEC to BIN format and display
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::convertDecToBinaryAndDisplay(int hoursRow, int minutesRow, int secondsRow)
      {
      bool oneBit;
      // A fast and efficient way to convert decimal to binary format and to set the individual LED colors
      // The original code ran in a hard-coded loop extracting on bit at a time from the decimal value
      // and setting the corresponding LED color based on whether the bit was 0 or 1
      // This code just does a bitwise AND and tests if the bit was non-zero, then sets the corresponding LED color.
      // I removed the loops as the number of LEDs is fixed in the hardware and doesn't need to be generic.
      // This results in a faster execution time and less code complexity, easier to maintain.
      // Using a MACRO to handle SERIAL_TIME compiles where the serial output is used, and 'binaryArray[]' is required.
      // Performing the assignment to 'binaryArray[]' then testing the result for non-zero to assign the colour
      #ifdef SERIAL_TIME
      // If SERIAL_TIME is defined, we need to keep track of the binary representation of the time
      #define SET_LEDS(led_num, value, bitmask, on_color, off_color) \
            leds[led_num] = (binaryArray[led_num] = ((value) & (bitmask))) ? on_color : off_color;
      #else
      #define SET_LEDS(led_num, value, bitmask, on_color, off_color) \
            leds[led_num] = ((value) & (bitmask)) ? on_color : off_color;
      #endif
      
      // H - upper row LEDs where the hour is displayed, left most bit, the MSB, is led 16, LSB is LED 12
      SET_LEDS(16, hoursRow,   0b00010000, OnColor[16],  OffColor[16]); // LED 16 - dot[4] for hour 16 (msb)
      SET_LEDS(15, hoursRow,   0b00001000, OnColor[15],  OffColor[15]); // LED 15 - dot[3] for hour  8      
      SET_LEDS(14, hoursRow,   0b00000100, OnColor[14],  OffColor[14]); // LED 14 - dot[2] for hour  4      
      SET_LEDS(13, hoursRow,   0b00000010, OnColor[13],  OffColor[13]); // LED 13 - dot[1] for hour  2      
      SET_LEDS(12, hoursRow,   0b00000001, OnColor[12],  OffColor[12]); // LED 12 - dot[0] for hour  1 (lsb)

      // M - middle row LEDs where the minute is displayed, left most bit, the MSB, is led 12, LSB is LED 6
      SET_LEDS(11, minutesRow, 0b00100000, OnColor[11], OffColor[11]); // LED 11 - dot[5] for minute 32 (msb)
      SET_LEDS(10, minutesRow, 0b00010000, OnColor[10], OffColor[10]); // LED 10 - dot[4] for minute 16      
      SET_LEDS( 9, minutesRow, 0b00001000, OnColor[ 9], OffColor[ 9]); // LED  9 - dot[3] for minute  8      
      SET_LEDS( 8, minutesRow, 0b00000100, OnColor[ 8], OffColor[ 8]); // LED  8 - dot[2] for minute  4      
      SET_LEDS( 7, minutesRow, 0b00000010, OnColor[ 7], OffColor[ 7]); // LED  7 - dot[1] for minute  2      
      SET_LEDS( 6, minutesRow, 0b00000001, OnColor[ 6], OffColor[ 6]); // LED  6 - dot[0] for minute  1 (lsb)

      // S - bottom row LEDs where the second is displayed, left most bit, the MSB, is led 5, LSB is LED 0
      SET_LEDS( 5, secondsRow, 0b00100000, OnColor[ 5], OffColor[ 5]); // LED  5 - dot[5] for second 32 (msb)
      SET_LEDS( 4, secondsRow, 0b00010000, OnColor[ 4], OffColor[ 4]); // LED  4 - dot[4] for second 16
      SET_LEDS( 3, secondsRow, 0b00001000, OnColor[ 3], OffColor[ 3]); // LED  3 - dot[3] for second  8
      SET_LEDS( 2, secondsRow, 0b00000100, OnColor[ 2], OffColor[ 2]); // LED  2 - dot[2] for second  4
      SET_LEDS( 1, secondsRow, 0b00000010, OnColor[ 1], OffColor[ 1]); // LED  1 - dot[1] for second  2
      SET_LEDS( 0, secondsRow, 0b00000001, OnColor[ 0], OffColor[ 0]); // LED  0 - dot[0] for second  1 (lsb)

      FastLED.show();
      }

   //################################################################################//
   // MELODY ALARM
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // During playing the alarm melody, time display function is disabled
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::playAlarm()
      {
      bool stopAlarm = LOW;
      int howManyTimes = 0;
      unsigned long millis_time_now = 0;

      // // Count how many notes are in the melody
      // int allNotes = sizeof(NoteDurations);

      while ((howManyTimes < alarmRepeatMax) & (stopAlarm == LOW))
         {
         for (int thisNote = 0; thisNote < noteDurationsSize; thisNote++)  // Changed from: allNotes
            {
            // To calculate the note duration, take one second divided by the note type.
            // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
            int noteDuration = 1000 / pgm_read_byte(&noteDurations[thisNote]);
            tone(PIEZO, pgm_read_word(&melodyAlarm[thisNote]), noteDuration);

            // To distinguish the notes, set a minimum time between them.
            // The note's duration + 30% seems to work well:
            int pauseBetweenNotes = noteDuration * 1.30;

            // Millis time start
            millis_time_now = millis();

            // Pause between notes
            while (millis() < millis_time_now + pauseBetweenNotes)
               {
               // Stop alarm melody and go to main menu
               if (checkS2() == HIGH)
                  {
                  // Prepare for escape to main menu
                  settingsLevel = 0;
                  settingsOption = 0;
                  stopAlarm = 1;

                  // Stop the tone playing
                  noTone(PIEZO);
                  }
               }

            // Escape to main menu
            if (stopAlarm == 1) break;

            // Stop the tone playing
            noTone(PIEZO);
            }
         howManyTimes++;
         }
      }

   //################################################################################//
   // CHECK BUTTONS
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Check if the S1 button has been pressed
   ////////////////////////////////////////////////////////////////////////////////////
   int BinaryClock::checkS1()
      {
      // Read the state of the push button into a local variable:
      bool currentreadS1 = digitalRead(S1);

      // Check to see if you just pressed the button
      // (i.e. the input went from LOW to HIGH), and you've waited long enough
      // since the last press to ignore any noise:

      // Check if button changed, due to noise or pressing:
      if (currentreadS1 != lastreadS1)
         {
         // Reset the debouncing timer
         lastDebounceTime = millis();
         }

      if ((millis() - lastDebounceTime) > debounceDelay)
         {
         // Whatever the reading is at, it's been there for longer than the debounce
         // delay, so take it as the actual current state:

         // If the button state has changed:
         if (currentreadS1 != S1state)
            {
            S1state = currentreadS1;

            // Return 1 only if the new button state is HIGH
            if (S1state == HIGH)
               {
               lastreadS1 = currentreadS1;
               return 1;
               }
            }
         }

      // Save S1 button state. Next time through the loop, it'll be the lastreadS2:
      lastreadS1 = currentreadS1;
      return 0;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Check if the S2 button has been pressed
   ////////////////////////////////////////////////////////////////////////////////////
   int BinaryClock::checkS2()
      {
      // Read the state of the push button into a local variable:
      bool currentreadS2 = digitalRead(S2);

      // Check to see if you just pressed the button
      // (i.e. the input went from LOW to HIGH), and you've waited long enough
      // since the last press to ignore any noise:

      // Check if button changed, due to noise or pressing:
      if (currentreadS2 != lastreadS2)
         {
         // Reset the debouncing timer
         lastDebounceTime = millis();
         }

      if ((millis() - lastDebounceTime) > debounceDelay)
         {
         // Whatever the reading is at, it's been there for longer than the debounce
         // delay, so take it as the actual current state:

         // If the button state has changed:
         if (currentreadS2 != S2state)
            {
            S2state = currentreadS2;

            // Return 1 only if the new button state is HIGH
            if (S2state == HIGH)
               {
               lastreadS2 = currentreadS2;
               return 1;
               }
            }
         }

      // Save S2 button state. Next time through the loop, it'll be the lastreadS2:
      lastreadS2 = currentreadS2;
      return 0;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Check if the S3 button has been pressed
   ////////////////////////////////////////////////////////////////////////////////////
   int BinaryClock::checkS3()
      {
      // Read the state of the push button into a local variable:
      bool currentreadS3 = digitalRead(S3);

      // Check to see if you just pressed the button
      // (i.e. the input went from LOW to HIGH), and you've waited long enough
      // since the last press to ignore any noise:

      // Check if button changed, due to noise or pressing:
      if (currentreadS3 != lastreadS3)
         {
         // Reset the debouncing timer
         lastDebounceTime = millis();
         }

      if ((millis() - lastDebounceTime) > debounceDelay)
         {
         // Whatever the reading is at, it's been there for longer than the debounce
         // delay, so take it as the actual current state:

         // If the button state has changed:
         if (currentreadS3 != S3state)
            {
            S3state = currentreadS3;

            // Return 1 only if the new button state is HIGH
            if (S3state == HIGH)
               {
               lastreadS3 = currentreadS3;
               return 1;
               }
            }
         }

      // Save S3 button state. Next time through the loop, it'll be the lastreadS3:
      lastreadS3 = currentreadS3;
      return 0;
      }

   //################################################################################//
   // SERIAL INFO
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Print Time in DEC & BIN format
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialTime()
      {
      #if SERIAL_TIME
      Serial << ("DEC:");
      Serial << ((hour(t) < 10) ? "0" : "") << _DEC(hour(t)) << (":");
      Serial << ((minute(t) < 10) ? "0" : "") << _DEC(minute(t)) << (":");
      Serial << ((second(t) < 10) ? "0" : "") << _DEC(second(t)) << ("  ");

      Serial << ("BIN:");
      for (int i = 16; i >= 0; i--)
         {
         if (i == 11 || i == 5) Serial << (" ");
         Serial << (binaryArray[i] ? "1" : "0"); // Print 1 or 0 for each LED
         }
      Serial << endl;
      #endif 
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show the Shield settings menu and alarm status
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialStartInfo()
      {
      #if SERIAL_SETUP
      Serial << F("-------------------------------------") << endl;
      Serial << F("------- BINARY CLOCK SHIELD ---------") << endl;
      Serial << F("----------- FOR ARDUINO -------------") << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("------------- Options ---------------") << endl;
      Serial << F("S1 - Time Settings ------------------") << endl;
      Serial << F("S2 - Disable Alarm Melody -----------") << endl;
      Serial << F("S3 - Alarm Settings -----------------") << endl;
      Serial << F("-------------------------------------");

      serialAlarmInfo();

      Serial << F("#####################################") << endl;
      Serial << F("START WITHIN 10 SECONDS -------------") << endl;

      // Show progress bar: #37 * 270ms = ~10s delay
      for (int i = 0; i < 37; i++)
         {
         delay(270);
         Serial << ("#");
         }

      Serial << endl << endl;
      #endif  
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show alarm settings
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialSettings()
      {
      #if SERIAL_SETUP
      if (settingsOption == 1)
         {
         Serial << endl << endl;
         Serial << F("-------------------------------------") << endl;
         Serial << F("---------- Time Settings ------------") << endl;
         Serial << F("-------------------------------------") << endl;
         }

      if (settingsOption == 3)
         {
         Serial << endl << endl;
         Serial << F("-------------------------------------") << endl;
         Serial << F("---------- Alarm Settings -----------") << endl;
         Serial << F("-------------------------------------") << endl;
         }

      if (settingsLevel == 1) Serial << F("--------------- Hour ----------------") << endl;
      if (settingsLevel == 2) Serial << F("-------------- Minute ---------------") << endl;
      if (settingsLevel == 3)
         {
         if (settingsOption == 1) Serial << F("-------------- Second ---------------") << endl;
         if (settingsOption == 3) Serial << F("-------------- ON/OFF ---------------") << endl;
         }

      Serial << F("S1 - Decrement ----------------------") << endl;
      Serial << F("S3 - Increment ----------------------") << endl;
      Serial << F("S2 - Save Current Settings Level ----") << endl;
      Serial << F("-------------------------------------") << endl;

      if (settingsLevel == 1) Serial << F("Current Hour: ") << countButtonPressed << (" ");
      if (settingsLevel == 2) Serial << F("Current Minute: ") << countButtonPressed << (" ");
      if (settingsLevel == 3)
         {
         if (settingsOption == 1) Serial << F("Current Second: ") << countButtonPressed << (" ");
         if (settingsOption == 3)
            {
            Serial << F("Alarm Status: ");
            Serial << (countButtonPressed == 2 ? "ON" : "");
            Serial << (countButtonPressed == 1 ? "OFF" : "");
            Serial << (" ");
            }
         }
      #endif       
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show the set alarm time and current alarm status
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialAlarmInfo()
      {
      #if SERIAL_SETUP
      Serial << endl << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("---- Alarm Time: ");
      Serial << (hourAlarm < 10 ? "0" : "") << hourAlarm << ":";
      Serial << (minuteAlarm < 10 ? "0" : "") << minuteAlarm;
      Serial << endl;
      Serial << F("-------------------------------------") << endl;

      Serial << F("---- Alarm Status: ");
      Serial << (alarmStatus == 2 ? "ON" : "");
      Serial << (alarmStatus == 1 ? "OFF" : "");
      Serial << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << endl;
      #endif   
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show current alarm status during settings
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialCurrentModifiedValue()
      {
      #if SERIAL_SETUP
      if ((settingsLevel == 3) & (settingsOption == 3))
         {
         Serial << (countButtonPressed == 2 ? "ON" : "");
         Serial << (countButtonPressed == 1 ? "OFF" : "");
         }
      else
         {
         Serial << countButtonPressed;
         }

      Serial << (" ");
      #endif   
      }
   }