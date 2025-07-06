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
// Note: (Chris-80 2025/07/03)
// =====
// This file has been adapted from the original Example; "11-BinaryClock-24H-RTCInterruptAlarmButtons.ino" file as published
// on the Binary Clock Shield for Arduino GitHub repository: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
// The original file was modified to be encapsulated in a class, BinaryClock. The class encapsulates all the functionality
// of the "Binary Clock Shield for Arduino." Modifications were made to support the ESP32 UNO platform and to allow
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

#include <FastLED.h>            // https://github.com/FastLED/FastLED
#include <DS3232RTC.h>          // https://github.com/JChristensen/DS3232RTC
#include <Streaming.h>          // https://github.com/janelia-arduino/Streaming                            
#include "pitches.h"            // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   // This is a Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
   //
   #define ESP32UNO             // If defined, the code will use ESP32 UNO board definitions
   //#define ATMELUNO           // If defined, the code will use ATMEL UNO board definitions

   // This determines if the menu and/or time are also displayed on the serial monitor.
   // If SERIAL_SETUP is defined, code of the serial menu to display is included in the project.
   // If SERIAL_TIME  is defined, code of the serial menu to display is included in the project.
   // The SERIAL_ON   is used to determine if the serial messages are displayed by default or not.
   //                 use the public methods 'set_isSerialSetup()' and 'set_isSerialTime()' to
   //                 enable/disable the serial output at runtime. SERIAL_ON only controls the initial state.
   #define SERIAL_SETUP  1      // If 1 (true) - serial setup ON, if false serial setup OFF
   #define SERIAL_TIME   1      // If 1 (true) - serial time  ON, if false serial time  OFF
   #define SERIAL_ON     0      // If 1 (true) - The serial message(s) are on by default (if enabled above).

   #if defined(ESP32_D1_R32) || defined(ESP32UNO) // ESP32 Wemos D1 R32 UNO board definitions
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

      // Debug PIN to print out over serial monitor if grounded at startup 
      // The SERIAL_MENU and/or SERIAL_TIME are defined (i.e. 1) in order
      // to compile the code and make them available. 
      #define DEBUG_PIN 27 // Set to 0 to disable the debug pin

   #elif defined(ATMELUNO)   // Standard Arduino UNO board definitions with the ATMEL chip
      // Arduino UNO ATMEL pin definitions
   #define INT           3         // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO         11        // The number of the Piezo pin
   #define LED_PIN       A3        // Data pin that LEDs data will be written out over

   #define S1  A2                  // Push buttons connected to the A0, A1, A2 Arduino pins
   #define S2  A1    
   #define S3  A0 
      // Debug PIN to print out over serial monitor if grounded at startup 
      // The SERIAL_MENU and/or SERIAL_TIME are defined (i.e. 1) in order
      // to compile the code and make them available. 
      #define DEBUG_PIN 0 // Set to 0 to disable the debug pin

   #else
      #error "Unsupported board. Please define the pin numbers for your board."
   #endif

   // The physical layout of the LEDs on the shield, one row each.
   #define NUM_HOUR_LEDS   5
   #define NUM_MINUTE_LEDS 6
   #define NUM_SECOND_LEDS 6
   #define NUM_LEDS (NUM_HOUR_LEDS + NUM_MINUTE_LEDS + NUM_SECOND_LEDS)

   #define LED_TYPE           WS2812B     // Datasheet: http://bit.ly/LED-WS2812B
   #define COLOR_ORDER         GRB        // For color ordering use this sketch: http://bit.ly/RGBCalibrate   

   #define DEFAULT_BRIGHTNESS        30   // The best tested LEDs brightness 20-60
   #define DEFAULT_ALARM_REPEAT       3   // How many times play the melody alarm

   #define DEFAULT_DEBUG_OFF_DELAY 5000   // Default delay to turn off serial monitor after pin goes HIGH
   #define HARDWARE_DEBUG (DEBUG_PIN > 0) && (SERIAL_TIME || SERIAL_SETUP)

   #define CA_ON LOW                      // The value when the button is pressed for CA connections
   #define CC_ON HIGH                     // The value when the button is pressed for CC connections
   #define CA_OFF HIGH                    // The value when the button is not pressed for CA connections
   #define CC_OFF LOW                     // The value when the button is not pressed for CC connections
   #define DEFAUT_DEBOUNCE_DELAY 50       // The default debounce delay in milliseconds for the buttons
   typedef struct alarmTime
      {
      int hour;          // Hour of the alarm (0-23)
      int minute;        // Minute of the alarm (0-59)
      int status;        // Status of the alarm: 0 - inactive, 1 - active
      } AlarmTime;

   /// @brief The structure that holds the state of a button.
   ///        It contains the pin number, current state, last read state, last read time
   ///        the onValue (the value when the button is pressed) for CC or CA connections.
   ///        The isPressed() method returns true if the button is currently pressed, false otherwise.
   typedef struct buttonState
      {
      uint8_t pin;                     // The button pin number: e.g. S1, S2, S3, etc.
      volatile int state;                       // The current state of the button: LOW or HIGH
      volatile int lastRead;                    // The last read state of the button: LOW or HIGH
      volatile unsigned long lastReadTime;      // The last time the button was read
      volatile unsigned long lastDebounceTime;  // The last debounce time in milliseconds
      int onValue;                     // The button value when pressed, HIGH (CC) or LOW (CA)
      bool isPressed() const           // True if the button is currently pressed, false otherwise
         { return (state == onValue); }  
      } ButtonState;

   class BinaryClock
      {
   public:
      /// @brief The method called to initialize the Binary Clock.
      ///        This has the same functionality of the Arduino setup() method.
      ///        Call this method before using the BinaryClock class.
      void setup();

      /// @brief The method called to run the Binary Clock.
      ///        This has the same functionality of the Arduino loop() method.
      ///        Call this method in the main loop of your program or run this
      ///        method in a separate thread that just loops forever.
      void loop();

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

      /// @brief The method called to set the time and/or alarm from the shield
      ///        The S1 button sets the Time, S3 sets the Alarm, S2 accepts
      ///        the current modified value and moves to the next line.
      ///        The S1 and S3 buttons increment/decrement the current modified value.
      void settingsMenu();

      /// @brief The method called to play the alarm melody.
      void playAlarm();

      //#################################################################################//
      // PROPERTIES
      //#################################################################################//
      /// @brief The method called to set the current 'Time' property.
      /// @param tm The tmElements_t structure containing the current time.
      /// @note The tmElements_t structure is defined in the Arduino Time Library.
      void set_Time(tmElements_t &value);

      /// @brief The method called to get the current 'Time' property.
      /// @return A reference to the tmElements_t structure containing the current time.
      /// @note The returned tmElements_t is overwritten on each call. Make a local copy if needed.
      tmElements_t& get_Time();

      /// @brief The method called to set the current 'Alarm' property.
      /// @param alarmTime The AlarmTime structure containing the alarm time and status.
      /// @note The AlarmTime structure contains the hour, minute, and status of the alarm
      ///       The status is 0 for inactive, 1 for active.
      ///       Hours are 0 to 23.
      void set_Alarm(AlarmTime &value);

      /// @brief The method called to get the current 'Alarm' property
      /// @return A reference to the AlarmTime structure containing the alarm time and status.
      /// @note The returned AlarmTime is overwritten on each call. Make a local copy if needed.
      AlarmTime& get_Alarm();

      /// @brief
      bool get_isSerialSetup() const;
      bool get_isSerialTime() const ;
      void set_isSerialSetup(bool value);
      void set_isSerialTime(bool value);

      void set_Brightness(byte value); 
      byte get_Brightness() const;

      void set_DebugOffDelay(unsigned long value);
      unsigned long get_DebugOffDelay() const;

      // Singleton pattern to ensure only one instance of BinaryClock
      static BinaryClock& get_Instance()
         {
         static BinaryClock instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }

   protected:
      BinaryClock();          // Constructor
      virtual ~BinaryClock(); // Destructor

      BinaryClock (const BinaryClock&) = delete;            // Disable copy constructor
      BinaryClock& operator=(const BinaryClock&) = delete;  // Disable assignment operator
      BinaryClock (BinaryClock&&) = delete;                 // Disable move constructor
      BinaryClock& operator=(BinaryClock&&) = delete;       // Disable move assignment operator

      void RTCinterrupt();
      void getAlarmTimeAndStatus(int& alarmHour, int& alarmMinute, int& alarmStatus);
      void getAlarmTimeAndStatus();
      void setAlarmTimeAndStatus();
      void getAndDisplayTime();
      void convertDecToBinaryAndDisplay(int bottomRow, int middleRow, int upperRow);
      void setNewTime();
      void setCurrentModifiedValue();
      void checkCurrentModifiedValueFormat();
      void saveCurrentModifiedValue();
      void displayCurrentModifiedValue();

      /// @brief Method to check if the button is pressed ON from OFF
      /// @param button - The ButtonState structure containing the button state, pin, type, etc.
      /// @return True if the button is pressed ON from OFF, false otherwise (button OFF or button ON and no change).
      /// @note The method returns false if the button is ON but has not changed state since the last read.
      ///       Check the button.isPressed() property to see if the button is currently pressed or not
      bool isButtonOnNew(ButtonState& button);

      #if SERIAL_TIME
      void serialTime();
      #endif

      #if SERIAL_SETUP
      void serialStartInfo();
      void serialSettings();
      void serialAlarmInfo();
      void serialCurrentModifiedValue();
      #endif

      #if HARDWARE_DEBUG
      void checkHardwareDebugPin();
      #endif

   public:         
      // These variables are intially set to the internal static melody and note durations arrays
      // They can be changed to use different melodies and note durations in the ESP32 flash memory.
      unsigned   *melodyAlarm;         // Pointer to the melody array
      int         melodySize;          // Size of the melody array
      byte       *noteDurations;       // Pointer to the note durations array
      int         noteDurationsSize;   // Size of the note durations array

      static CRGB OnColor [NUM_LEDS]     PROGMEM; // Colors for the LEDs when ON
      static CRGB OffColor[NUM_LEDS]     PROGMEM; // Colors for the LEDs when OFF

   protected:
      DS3232RTC RTC;                               // Create RTC object

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
      static const unsigned long NoteDurations[] PROGMEM; // Note durations array, unsigned long array (64 bits)
      static const unsigned      MelodyAlarm[] PROGMEM;   // Melody for alarm, unsigned integer array (32 bits)
      static const int           MelodySize; // Size of the melody array
      static const int           NoteDurationsSize; // Size of the note durations array

   private:
      ButtonState buttonS1    = { .pin = S1, .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = HIGH };
      ButtonState buttonS2    = { .pin = S2, .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = HIGH };
      ButtonState buttonS3    = { .pin = S3, .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = HIGH };
      ButtonState buttonDebug = { .pin = DEBUG_PIN, .state = CA_OFF, .lastRead = CA_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = CA_ON };

      int countButtonPressed;                // Counter for button pressed
      static volatile bool RTCinterruptWasCalled;   // Flag for RTC interrupt was called

      unsigned long debounceDelay = DEFAUT_DEBOUNCE_DELAY; // The debounce time for a button press.

      // Variables that store the current settings option
      int settingsOption = 0;               // Time = 1, Alarm = 3  
      int settingsLevel = 0;                // Hours = 1, Minutes = 2, Seconds / On/Off Alarm = 3

      // Variables that store the current alarm time and status
      int hourAlarm = 0;
      int minuteAlarm = 0;
      int alarmStatus = 0;
      AlarmTime tempAlarmTime = {0, 0, 0}; // Temporary alarm time 
      tmElements_t tempTime = {0, 0, 0, 0, 0, 0, 0}; // Temporary time structure

      int alarmRepeatMax = DEFAULT_ALARM_REPEAT;   // Maximum alarm repeat count
      int alarmRepeatCount = 0;                    // Current alarm repeat count
      byte brightness = DEFAULT_BRIGHTNESS;        // Brightness of the LEDs, 0-255

      bool isSerialSetup = (SERIAL_SETUP == 1) && (SERIAL_ON == 1); // Serial setup flag
      bool isSerialTime  = (SERIAL_TIME  == 1) && (SERIAL_ON == 1); // Serial time flag   

      unsigned long debugDelay = DEFAULT_DEBUG_OFF_DELAY;
      };
   }
#endif