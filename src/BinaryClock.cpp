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
#include <RTCLib.h>             // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>          // https://github.com/janelia-arduino/Streaming                            
#include "pitches.h"            // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   // This is a Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
   //

   // Notes in the melody: (70)
   const unsigned BinaryClock::MelodyAlarm[] PROGMEM =
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

   #define NOTE_MS(N) (1000 / N) // Convert note duration to milliseconds
   // Note durations: 4 = quarter note, 8 = eighth note, etc.:
   // Some notes durations have been changed (1, 3, 6) to make them sound better
   const unsigned long BinaryClock::NoteDurations[] PROGMEM =
         {
         NOTE_MS(2), NOTE_MS(2), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6),
         NOTE_MS(1), NOTE_MS(2), NOTE_MS(2), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(3),
         NOTE_MS(6), NOTE_MS(1), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(4), NOTE_MS(4),
         NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(3), NOTE_MS(4), NOTE_MS(2), NOTE_MS(4), NOTE_MS(4),
         NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6),
         NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(1), NOTE_MS(2), NOTE_MS(3), NOTE_MS(8), NOTE_MS(2),
         NOTE_MS(4), NOTE_MS(4), NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(4), NOTE_MS(4), NOTE_MS(2),
         NOTE_MS(4), NOTE_MS(4), NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(4), NOTE_MS(4), NOTE_MS(2),
         NOTE_MS(3), NOTE_MS(8), NOTE_MS(2), NOTE_MS(3), NOTE_MS(8), NOTE_MS(1),
         };

   // Calculate the number of elements in each array, then we validate they are the same size to catch definition errors.
   const int BinaryClock::MelodySize = sizeof(BinaryClock::MelodyAlarm) / sizeof(BinaryClock::MelodyAlarm[0]); 
   const int BinaryClock::NoteDurationsSize = sizeof(BinaryClock::NoteDurations) / sizeof(BinaryClock::NoteDurations[0]); 
   
   CRGB BinaryClock::leds[NUM_LEDS] = {};        // Array of LED colors to display the current time
   bool BinaryClock::binaryArray[NUM_LEDS] = {}; // Serial Debug: Array for binary representation of time

   // Default: Colors for the LEDs when ON, Seconds, Minutes and Hours
   CRGB BinaryClock::OnColor[NUM_LEDS] = 
         {
         CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,    // Seconds
         CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,  // Minutes
         CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Blue                 // Hours
         };
   // Default: Colors for the hours LEDs when OFF (Usually Black or No Power (i.e. OFF), Seconds, Minutes and Hours)
   CRGB BinaryClock::OffColor[NUM_LEDS] = 
         {
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Seconds
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Minutes
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black                // Hours
         };

   // Flag to indicate the RTC interrupt was called (Class Global)
   volatile bool BinaryClock::RTCinterruptWasCalled;

   // When the SERIAL_SETUP code is remove, redefine the method calls to be whitespace only
   // This allows the code to compile without the serial setup code, but still allows the methods 
   // to be "called" in the code without causing compilation errors (Must return void to work).
   #if SERIAL_SETUP != true
   #define serialStartInfo()
   #define serialSettings()
   #define serialAlarmInfo()
   #define serialCurrentModifiedValue()
   #endif 

   // When the SERIAL_TIME code is remove, redefine the method calls to be whitespace only
   #if SERIAL_TIME != true
   #define serialTime() 
   #endif

   //################################################################################//
   // RTC LIBRARY PLUS - EXTENDED FUNCTIONALITY
   //################################################################################//

   uint8_t  RTCLibPlusDS3231::rawRead(uint8_t reg)
      {
      return read_register(reg);  // Call the base class method to read a register
      }

   void RTCLibPlusDS3231::rawWrite(uint8_t reg, uint8_t value)
      {
      write_register(reg, value);  // Call the base class method to write a register
      }
      

   //################################################################################//
   // SETUP
   //################################################################################//
   void BinaryClock::setup()
      {
      #if SERIAL_SETUP || SERIAL_TIME
      Serial.begin(115200);
      delay(10);
      Serial.println("Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com");
         #if DEBUG_PIN > 0
         Serial << "Serial Menu: " << (isSerialSetup ? "ON" : "OFF") << "; Serial Time: " << (isSerialTime ? "ON" : "OFF") << "\n";
         #endif
      #endif

      assert(melodySize == noteDurationsSize);  // Ensure the melody and note durations arrays are the same size

      //#####################################################################//
      //#            Initialize the RTC library and set up the RTC          #//
      //#####################################################################//
      RTC.begin();

      // Configure an interrupt on the falling edge from the RTC INT/SQW output
      pinMode(RTC_INT, INPUT_PULLUP);
      // Attach the interrupt to the member RTCinterrupt function using a lambda
      attachInterrupt(
            digitalPinToInterrupt(RTC_INT),
            []() { BinaryClock::get_Instance().RTCinterrupt(); },
            FALLING);

      // Get the alarms stored in the RTC memory.
      alarm1.time = RTC.getAlarm1();
      alarm2.time = RTC.getAlarm2();
      // Clear the alarm status flags 'A1F' and 'A2F'
      RTC.clearAlarm(alarm1.number);   // TODO: ?????? Fix to refect inistial value.
      RTC.clearAlarm(alarm2.number);

      // Enable 1 Hz square wave RTC SQW output
      RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_SquareWave1Hz);
      //=====================================================================//

      // Initialize the buttons pins as an input
      pinMode(buttonS1.pin, INPUT_PULLDOWN);
      pinMode(buttonS2.pin, INPUT_PULLDOWN);
      pinMode(buttonS3.pin, INPUT_PULLDOWN);

      getAlarmTimeAndStatus();
      if (isSerialSetup) { serialStartInfo(); }

      //#####################################################################//
      //#            Initialize the FastLED library                         #//
      //#####################################################################//

      // Important power-up safety delay
      delay(3000);

      // Limit my draw to 450mA at 5V of power draw
      FastLED.setMaxPowerInVoltsAndMilliamps(5, 450);

      // Initialize LEDs
      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
      FastLED.setBrightness(brightness);
      FastLED.clear();  // Clear the LEDs to start with no lights on
      FastLED.show();   // Show the cleared state
      digitalWrite(BUILTIN_LED, LOW);
      }

   //################################################################################//
   // MAIN LOOP
   //################################################################################//
   void BinaryClock::loop()
      {
      #if HARDWARE_DEBUG
      checkHardwareDebugPin();
      #endif

      settingsMenu();   // Check if the settings menu is active and handle button presses

      if (BinaryClock::RTCinterruptWasCalled && (settingsOption == 0))   // Display time but not during settings
         {
         BinaryClock::RTCinterruptWasCalled = false;       // Clear the interrupt flag
         getAndDisplayTime();                              // Get time from RTC, convert to binary format and display on LEDs
         
         #if SERIAL_TIME
         if (isSerialTime) { serialTime(); }               // Use serial monitor for showing current time 
         #endif

         if (RTC.alarmFired(alarm2.number) & (alarm2.status > 0))
            {
            #if SERIAL_TIME
            if (isSerialTime) { Serial << "   ALARM!\n"; }
            #endif  

            playAlarm();
            }
         }
      }

   //################################################################################//
   /// @brief Default Constructor for the BinaryClock class.
   /// This initializes the button states, settings options, and brightness.
   /// It also assigns the melody and note durations arrays to the class members.
   BinaryClock::BinaryClock() :
         countButtonPressed(0),                 // Initialize the button counter
         settingsOption(0),                     // Initialize the settings option
         settingsLevel(0),                      // Initialize the settings level
         brightness(DEFAULT_BRIGHTNESS)         // Initialize the brightness
      {
      melodyAlarm = (unsigned *)MelodyAlarm;    // Assign the melody array to the pointer
      melodySize = MelodySize;                  // Assign the size of the melody array
      noteDurations = (byte *)NoteDurations;    // Assign the note durations array to the pointer
      noteDurationsSize = NoteDurationsSize;    // Assign the size of the note

      #if HARDWARE_DEBUG
      pinMode(DEBUG_SETUP_PIN, INPUT_PULLUP);  // Set the debug pin as input with pull-up resistor
      pinMode(DEBUG_TIME_PIN,  INPUT_PULLUP);  // Set the debug pin as input with pull-up resistor
      if (get_isSerialSetup())                 // If the serial setup flag is set
         {
         Serial.println("Serial Setup flag is ON. Serial Setup will be enabled.");
         isSerialSetup = true;                  // Enable serial setup
         }
      if (digitalRead(DEBUG_TIME_PIN) == CA_ON)   // If the debug pin is grounded
         {
         Serial.println("Debug pin is ON (grounded), Serial Time will be enabled.");
         isSerialTime = true;                   // Enable serial time
         }
      #endif
      }
   
   //################################################################################//
   /// @brief Destructor for the BinaryClock class.
   /// This destructor is empty as there is no dynamic memory allocation in this class.
   BinaryClock::~BinaryClock()
      {
      // Destructor
      // No dynamic memory allocation, nothing to clean up
      }

   //################################################################################//
   /// @brief Interrupt handler for the RTC. Just set a flag to indicate the interrupt was called.
   /// This function is called when the RTC INT/SQW output goes low, indicating one second has passed.
   void BinaryClock::RTCinterrupt()
      {
      BinaryClock::RTCinterruptWasCalled = true;
      }

   void BinaryClock::set_Time(DateTime &value)
      {
      RTC.adjust(value);    // Set the time in the RTC
      }

   DateTime BinaryClock::get_Time()
      {
      return RTC.now();
      }

   void BinaryClock::set_Alarm(AlarmTime& value)
      {
      if (value.number < ALARM_1 || value.number > ALARM_2) { return; }

      // Set the alarm time and status in the RTC
      if (value.status > 0)
         {
         // NOTE: (Chris-70, 2025/07/05)
         // The current version of Adafruit's RTCLib (2.1.4) does not allow setting the
         // Alarm interrupt registers A1IE and A2IE when the INTCN bit is set to 0.
         // This means we must disable the 1Hz SQ Wave, set the bits and then re-enable the SQ Wave.
         // This shouldn't impact the LED display but it might generate an additional interrupt.
         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);
         // If the alarm status is +ve, set the alarm to sound at 'hour:minute' each day.
         if (value.number == ALARM_1)
            {
            RTC.setAlarm1(value.time, Ds3231Alarm1Mode::DS3231_A1_Hour);
            }
         else if (value.number == ALARM_2)
            {  
            RTC.setAlarm2(value.time, Ds3231Alarm2Mode::DS3231_A2_Hour);
            }
         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_SquareWave1Hz);
         }
      else if (value.status == 0)
         {
         RTC.disableAlarm(value.number);
         }
      else { ; } // Ignore bad input status
      }

   AlarmTime BinaryClock::get_Alarm(int number)
      {
      AlarmTime result;
      if (number == ALARM_1)
         {
         alarm1.time = RTC.getAlarm1();
         alarm1.status = RTC.rawRead(DS3231_CONTROL) & DS3231_ALARM1_STATUS_MASK;
         result = alarm1;
         }
      if (number == ALARM_2) 
         {
         alarm2.time = RTC.getAlarm2();
         alarm2.status = (RTC.rawRead(DS3231_CONTROL) & DS3231_ALARM2_STATUS_MASK) >> 1;
         result = alarm2;
         }

      return result;
      }

   bool BinaryClock::get_isSerialSetup() const
      {
      #if SERIAL_SETUP
      return isSerialSetup;
      #else
      return false;
      #endif
      }

   void BinaryClock::set_isSerialSetup(bool value)
      {
      #if SERIAL_SETUP
      isSerialSetup = value;
      #endif
      }

   bool BinaryClock::get_isSerialTime() const
      {
      #if SERIAL_TIME
      return isSerialTime;
      #else
      return false;
      #endif
      }

   void BinaryClock::set_isSerialTime(bool value)
      {
      #if SERIAL_TIME
      isSerialTime = value;
      #endif
      }

   void BinaryClock::set_Brightness(byte value)
      {
      brightness = value;
      FastLED.setBrightness(brightness); // Set the brightness of the LEDs
      }

   byte BinaryClock::get_Brightness() 
      {
      brightness = FastLED.getBrightness();
      return brightness;
      }

   void BinaryClock::set_DebugOffDelay(unsigned long value)
      {
      debugDelay = value;
      }

   unsigned long BinaryClock::get_DebugOffDelay() const
      {
      return debugDelay;
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
   
   /// @brief This function handles the settings menu for the Binary Clock.
   /// It allows the user to set the time and alarm, modify the current value, and
   /// save the modified value. The settings menu is navigated using the buttons S1, S2, and S3.
   void BinaryClock::settingsMenu()
      {
      // Main menu
      if ((settingsOption == 0) & (settingsLevel == 0))
         {
         // Time settings
         if (isButtonOnNew(buttonS1))
            {
            DateTime temp = RTC.now();              // Read time from RTC 
            t = temp.secondstime();                 // Convert to time_t format
            settingsOption = 1;                     // Set time option settings
            settingsLevel = 1;                      // Set hour level settings
            setCurrentModifiedValue();              // Assign hours for modify +/- 
            if (isSerialSetup) { serialSettings(); }// Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current hour on LEDs
            }

         // Alarm settings
         if (isButtonOnNew(buttonS3))
            {
            getAlarmTimeAndStatus();                // Read alarm time and status from RTC
            AlarmTime temp = get_Alarm(ALARM_2);    // Get the second alarm time and status

            settingsOption = 3;                     // Set Alarm time option settings
            settingsLevel = 1;                      // Set hour level settings
            setCurrentModifiedValue();              // Assign hours for modify +/-
            if (isSerialSetup) { serialSettings(); }// Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current alarm hour on LEDs 
            }
         }

      // Any settings option level except main menu
      if (settingsLevel != 0)
         {
         // Decrement - if the buttonS1 was just pressed
         if (isButtonOnNew(buttonS1))
            {
            countButtonPressed--;                  // Decrement current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();     // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();         // Display current modified value on LEDs 
            if (isSerialSetup) { serialCurrentModifiedValue(); }  // Use serial monitor for showing settings
            }

         // Increment - if the buttonS3 was just pressed
         if (isButtonOnNew(buttonS3)) 
            {
            digitalWrite(BUILTIN_LED, HIGH); // Turn on the built-in LED to indicate settings mode
            countButtonPressed++;                  // Increment current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();     // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();         // Display current modified value on LEDs  
            if (isSerialSetup) { serialCurrentModifiedValue(); }  // Use serial monitor for showing settings
            }

         // Save if buttonS2 was just pressed
         if (isButtonOnNew(buttonS2))
            {
            saveCurrentModifiedValue();            // Save current value e.g. hour, minute, second, alarm status     
            settingsLevel++;                       // Go to next settings level - hour => minute => second / alarm status 

            if (settingsLevel > 3)                 // If escape from settings then return to main menu
               {
               if (settingsOption == 1)            // If you were in the process of setting the time:   
                  {
                  setNewTime();                    // Save time to the RTC
                  }

               if (settingsOption == 3)            // If you were in the process of setting the alarm: 
                  {
                  setAlarmTimeAndStatus();         // Save time and alarm status to the RTC    
                  }

               serialAlarmInfo();                  // Show the time and alarm status info when you exit to the main menu
               settingsLevel = 0;                  // Escape to main menu  
               settingsOption = 0;                 // Escape to main menu
               }
            else                                   // If you do not go to the main menu yet
               {
               checkCurrentModifiedValueFormat();  // Check if the value has exceeded the range e.g minute = 60 and correct                  
               setCurrentModifiedValue();          // Assign next variable for modify +/- hour => minute => second / alarm status
               displayCurrentModifiedValue();      // Display current modified value on LEDs                
               if (isSerialSetup) { serialSettings(); }  // Use serial monitor for showing settings
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
         if (settingsLevel == 1)  countButtonPressed = time.hour();
         if (settingsLevel == 2)  countButtonPressed = time.minute();
         if (settingsLevel == 3)  countButtonPressed = time.second();
         }

      // Alarm time and alarm status 
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)  countButtonPressed = alarm2.time.hour();
         if (settingsLevel == 2)  countButtonPressed = alarm2.time.minute();
         if (settingsLevel == 3)  countButtonPressed = alarm2.status + 1;
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
         if (settingsLevel == 1) time = DateTime(time.year(), time.month(), time.day(), countButtonPressed, time.minute(), time.second());
         if (settingsLevel == 2) time = DateTime(time.year(), time.month(), time.day(), time.hour(), countButtonPressed, time.second());
         if (settingsLevel == 3) time = DateTime(time.year(), time.month(), time.day(), time.hour(), time.minute(), countButtonPressed);
         }

      // Alarm time and alarm status
      if (settingsOption == 3)
         {
         if (settingsLevel == 1) alarm2.time = DateTime(alarm2.time.year(), alarm2.time.month(), alarm2.time.day(), countButtonPressed, alarm2.time.minute(), alarm2.time.second());
         if (settingsLevel == 2) alarm2.time = DateTime(alarm2.time.year(), alarm2.time.month(), alarm2.time.day(), alarm2.time.hour(), countButtonPressed, alarm2.time.second());
         if (settingsLevel == 3) alarm2.status = countButtonPressed - 1; // Convert 1/2 to 0/1 for the alarm status
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
      alarm2 = get_Alarm(ALARM_2); // Get the second alarm time and status
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Set alarm time and status
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::setAlarmTimeAndStatus()
      {
      set_Alarm(alarm2); // Set the second alarm time and status in the RTC
      }

   //################################################################################//
   // RTC TIME & BINARY FORMAT
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Write time to RTC
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::setNewTime() { set_Time(time); }

   ////////////////////////////////////////////////////////////////////////////////////
   // Get time from RTC and convert to BIN format
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::getAndDisplayTime()
      {
      time = get_Time();
      convertDecToBinaryAndDisplay(time.hour(), time.minute(), time.second());
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Convert values from DEC to BIN format and display
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::convertDecToBinaryAndDisplay(int hoursRow, int minutesRow, int secondsRow)
      {
      // A fast and efficient way to convert decimal to binary format and to set the individual LED colors
      // The original code ran in a hard-coded loop extracting on bit at a time from the decimal value
      // and setting the corresponding LED color based on whether the bit was 0 or 1
      // This code just does a bitwise AND and tests if the bit was non-zero, then sets the corresponding LED color.
      // I removed the loops as the number of LEDs is fixed in the hardware and doesn't need to be generic.
      // This results in a faster execution time and less code complexity, easier to maintain.
      // Using a MACRO to handle SERIAL_TIME compiles where the serial output is used, and 'binaryArray[]' is required.
      // Performing the assignment to 'binaryArray[]' then testing the result for non-zero to assign the colour
      #if SERIAL_TIME
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
      unsigned long millis_time_now = 0;
      unsigned long noteDuration;

      for (int i = 0; i < alarmRepeatMax; i++)
         {
         for (int thisNote = 0; thisNote < noteDurationsSize; thisNote++)  // Changed from: allNotes
            {
            noteDuration = noteDurations[thisNote];
            // Create the tone with the note frequency and duration for the current note
            tone(PIEZO, melodyAlarm[thisNote], noteDuration);

            // To distinguish the notes, set a minimum time between them.
            // The note's duration + 30% seems to work well; using 31.25% to remove floating point math (25% + 6.25%).
            int pauseBetweenNotes = noteDuration + (noteDuration >> 2) + (noteDuration >> 4); // 1.3125 instead of 1.30;

            // Millis time start, time 0 for this note pause
            millis_time_now = millis();

            // Pause between notes
            while (millis() < millis_time_now + pauseBetweenNotes)
               {
               // Stop alarm melody and go to main menu
               if (isButtonOnNew(buttonS2))
                  {
                  // Prepare for escape to main menu
                  settingsLevel = 0;
                  settingsOption = 0;

                  // Stop the tone playing
                  noTone(PIEZO);

                  // Exit we are done. Escape to main menu
                  return;
                  }
               }

            // Stop the tone playing between notes
            noTone(PIEZO);
            }
         }
      }

   //################################################################################//
   // CHECK BUTTONS
   //################################################################################//

   bool BinaryClock::isButtonOnNew(ButtonState &button)
      {
      bool result = false;

      // Read the state of the push button into a local variable:
      int currentread = digitalRead(button.pin);
      unsigned long currentReadTime = millis();

      // for the first read of the button (i.e. lastReadTime == 0) we will treat this
      // as a new button state and force the code to proceed as if this was a state change.
      if (button.lastReadTime == 0)
         {
         button.state = !currentread;
         button.lastRead = currentread;
         Serial << currentReadTime << " Button " << button.pin << " initial state is "
            << (currentread == button.onValue ? "ON" : "OFF") << " (" << currentread << ")" << endl;
         }

      // Check to see if you just pressed the button
      // (i.e. the input went from LOW to HIGH), and you've waited long enough
      // since the last press to ignore any noise:

      // Check if button changed, due to noise or pressing:
      if (currentread != button.lastRead)
         {
         // Reset the debouncing timer
         button.lastDebounceTime = currentReadTime;
         }

      // Whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:
      if ((currentReadTime - button.lastDebounceTime) > debounceDelay)
         {
         // If the button state has changed:
         if (currentread != button.state)
            {
            Serial << currentReadTime << " Button " << button.pin << " state changed from " 
                   << (button.state == button.onValue? "ON" : "OFF") << " (" << button.state 
                   << ") to " << (currentread == button.onValue? "ON" : "OFF") 
                   << " (" << currentread << ")" << " at " << currentReadTime << endl;
            button.state = currentread;
            button.lastReadTime = currentReadTime; // Save the time of the last state change

            // Return 1 only if the new button state is HIGH
            if (button.state == button.onValue)
               {
               result = true;
               }
            }
         }

      // Save button state. Next time through the loop, it'll be the lastread:
      button.lastRead = currentread;
      return result;
      }

   //################################################################################//
   // SERIAL INFO
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Print Time in DEC & BIN format
   ////////////////////////////////////////////////////////////////////////////////////
   #if SERIAL_TIME
   void BinaryClock::serialTime()
      {
      strncpy(buffer, "hh:mm:ss", sizeof(buffer)); 
      Serial << ("Time DEC: ") << (time.toString(buffer)) << ("  ") << ("BIN: ");

      for (int i = NUM_LEDS - 1; i >= 0; i--)
         {
         if (i == 11 || i == 5) Serial << (" ");
         Serial << (binaryArray[i] ? "1" : "0"); // Print 1 or 0 for each LED
         }
      Serial << endl;
      }
   #endif 

   #if SERIAL_SETUP
   ////////////////////////////////////////////////////////////////////////////////////
   // Show the Shield settings menu and alarm status
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialStartInfo()
      {
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
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show alarm settings
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialSettings()
      {
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
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show the set alarm time and current alarm status
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialAlarmInfo()
      {
      strncpy(buffer, "hh:mm", sizeof(buffer));
      Serial << endl << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("---- Alarm Time: ");
      Serial << (alarm2.time.toString(buffer));
      Serial << endl;
      Serial << F("-------------------------------------") << endl;

      Serial << F("---- Alarm Status: ");
      Serial << (alarm2.status ? "ON" : "OFF");
      Serial << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << endl;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show current alarm status during settings
   ////////////////////////////////////////////////////////////////////////////////////
   void BinaryClock::serialCurrentModifiedValue()
      {
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
      }
   #endif // SERIAL_SETUP
    
   #if HARDWARE_DEBUG
   void BinaryClock::checkHardwareDebugPin()
      {
      static bool isSerialSetupOn = DEFAULT_SERIAL_SETUP;

      if (isButtonOnNew(buttonDebugSetup)) 
         {
         Serial << F("Hardware debug pin is pressed. Serial setup output toggled from: ") 
                << (isSerialSetupOn ? "ON" : "OFF") << " to "
                << (!isSerialSetupOn? "ON" : "OFF") << endl;
         set_isSerialSetup(!isSerialSetupOn);
         isSerialSetupOn = !isSerialSetupOn;         
         }

      set_isSerialSetup(true); // Set the serial setup flag to true
      // Check if the hardware debug pin is set
      if (isButtonOnNew(buttonDebugTime)) 
         {
         set_isSerialTime(true); // Set the serial time flag to true
         // buttonDebug.lastReadTime = millis(); // Update the last debug time
         Serial << F("Hardware debug pin is set. Serial output enabled.") << endl;
         }
      else if (isSerialTime && !buttonDebugTime.isPressed() && ((millis() - buttonDebugTime.lastReadTime) > debugDelay)) 
         {
         set_isSerialTime(false); // Reset the serial time flag
         Serial << F("Hardware debug pin is not set. Serial output disabled. ") << (millis() - buttonDebugTime.lastReadTime) 
               << " millis: " << millis() << " Last Read: " << buttonDebugTime.lastReadTime << endl;
         }
      }
   #endif
   }  // END OF NAMESPACE BinaryClockShield && BINARY CLOCK CLASS code
   //################################################################################//
   