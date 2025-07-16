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
// of the "Binary Clock Shield for Arduino by Marcin Saj." Modifications were made to support the ESP32 UNO platform and to
// allow greater flexibility by the user at runtime, such as the color selection and melodies used for the alarm.
// 
// The goal of using an ESP32 based UNO board was to allow the RTC to be connected to a NTP server over WiFi. The code
// for the WiFi connection is encapsulated in its own class, WiFiClock, which is not included in this file. It uses 
// WPS to connect to a WiFi network and stores the credentials in the ESP32 flash memory so future connections are
// made automatically without user intervention. The WiFi connection also allows the user to change the LED colors
// and melodies used for the alarm at runtime, without needing to recompile the code. 
//

#ifndef _BINARY_CLOCK_RTC_24_ALARM_BUTTONS_
#define _BINARY_CLOCK_RTC_24_ALARM_BUTTONS_

#include <Arduino.h>            // Arduino core library
#include <tuple>                 // Tuple library for C++11
#include <cstdint>               // C++ standard library for fixed-width integer types


#include <FastLED.h>          // https://github.com/FastLED/FastLED
#include <RTCLib.h>           // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>        // https://github.com/janelia-arduino/Streaming                            
#include "pitches.h"          // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   // This is a Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
   // 
   // The following are defines for the currently supported boards. One must be used to compile.
   // Add you own UNO style board definitions below for a different UNO sized board you have.
   //
   // #define ESP32_D1_R32_UNO   // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions
   // #define METRO_ESP32_S3     // If defined, the code will use Adafruit Metro ESP32-S3 board definitions 
   // #define ESP32_S3_UNO       // If defined, the code will use ESP32-S3 UNO board definitions
   // #define ATMEL_UNO_R3       // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions
   // 
   // Debug Time PIN to print out the current time over serial monitor (if grounded)
   // The SERIAL_MENU and/or SERIAL_TIME_CODE are defined (i.e. 1) in order
   // to compile the code and make them available. They can also be set
   // in the software: 'void BinaryClock::set_isSerialSetup(bool value)' and
   // 'void BinaryClock::set_isSerialTime(bool value)' methods.
   // The debug time and setup pins are used to enable/disable the serial output at runtime.
   // without the need to change the software. The Serial Time is a switch to enable/disable 
   // the serial time display, displays while switch is ON. The Serial Setup is a momentary button
   // to toggle enable/disable the serial setup display. 
   // When the PIN value is -1 (-ve) the associated code is removed.

   //################################################################################//
   //             Defines for the different UNO sized boards                         //
   //################################################################################//
   // Generic AliExpress copy of Wemos D1 R32 ESP32 based UNO board (validate against the board you receive)
   #if defined(ESP32_D1_R32_UNO)     // ESP32 Wemos D1 R32 UNO board definitions
      #define ESP32UNO               // Define ESP32UNO as a common base architecture for ESP32 UNO boards

      // ESP32 UNO pin definitions
      #define RTC_INT           25   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
      #define PIEZO             23   // The number of the Piezo pin
      #define LED_PIN           15   // Data pin that LEDs data will be written out. Requires ESP32UNO board modification to use pin 32
                                     // If you use ESP32UNO board, you need to modify the ESP32UNO board by removing the connector at pin 34
                                     // Solder a jumper wire from PIN 32 to the LED pin on the shield. 
                                     // ESP32UNO PIN 34 is Read-Only and cannot be used for output.

      #define S1                35   // Push buttons connected to the A2, A1, A0 Arduino pins (CC)
      #define S2                 4    
      #define S3                 2 

      #define DEBUG_SETUP_PIN   16   // Set to -1 to disable the Serial Setup display control by H/W (CA)
      #define DEBUG_TIME_PIN    27   // Set to -1 to disable the Serial Time display control by H/W (CA)

   // Adafruit Metro ESP32-S3 board (https://learn.adafruit.com/adafruit-metro-esp32-s3)
   #elif defined(METRO_ESP32_S3)
      #define ESP32UNO

      // Adafruit Metro ESP32-S3 pin definitions
      #define RTC_INT            3   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
      #define PIEZO             11   // The number of the Piezo pin
      #define LED_PIN           A3   // Data pin that LEDs data will be written out.

      #define S1                A2   // Push buttons connected to the A2, A1, A0 Arduino pins (CC)
      #define S2                A1    
      #define S3                A0 

      #define DEBUG_SETUP_PIN    5   // Set to -1 to disable the Serial Setup display control by H/W (CA)
      #define DEBUG_TIME_PIN     6   // Set to -1 to disable the Serial Time display control by H/W (CA)

   // Generic AliExpress ESP32-S3 UNO board definitions (validate against the board you receive)
   #elif defined(ESP32_S3_UNO)
      #define ESP32UNO

      // AliExpress ESP32-S3 UNO pin definitions
      #define RTC_INT           17   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
      #define PIEZO             11   // The number of the Piezo pin
      #define LED_PIN            6   // Data pin that LEDs data will be written out.

      #define S1                 7   // Push buttons connected to the A2, A1, A0 Arduino pins (CC)
      #define S2                 1    
      #define S3                 2 

      #define DEBUG_SETUP_PIN   10   // Set to -1 to disable the Serial Setup display control by H/W (CA)
      #define DEBUG_TIME_PIN    11   // Set to -1 to disable the Serial Time display control by H/W (CA)

   #elif defined(ATMEL_UNO_R3)   // Standard Arduino UNO R3 board definitions using the ATMEL 328 chip
      // Arduino UNO ATMEL 328 based pin definitions
      #define RTC_INT            3   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
      #define PIEZO             11   // The number of the Piezo pin
      #define LED_PIN           A3   // Data pin that LEDs data will be written out over

      #define S1                A2   // Push buttons connected to the A2, A1, A0 Arduino pins
      #define S2                A1    
      #define S3                A0 

      #define DEBUG_SETUP_PIN    5  // Set to -1 to disable the Serial Setup display control by H/W
      #define DEBUG_TIME_PIN     6  // Set to -1 to disable the Serial Time display control by H/W

      // These defines are used in the code to satisfy the UNO compiler
      #define UNO_ARRAY_STATIC_CONST      static const
      #define ESP32_INPUT_PULLDOWN        INPUT
      #define BINARY_CLOCK_ARRAY_MEMBER

   #else
      #error "Unsupported board. Please define the pin numbers for your board."
   #endif

   #ifdef ESP32UNO
      // These defines are used to enable redefinition when compiling for UNO R3
      // These are the definitions used with a standard C++ compiler.
      #define UNO_ARRAY_STATIC_CONST
      #define ESP32_INPUT_PULLDOWN        INPUT_PULLDOWN
      #define BINARY_CLOCK_ARRAY_MEMBER   BinaryClock::
   #endif 
   //################################################################################//

   // The physical layout of the LEDs on the shield, one row each.
   #define NUM_HOUR_LEDS   5
   #define NUM_MINUTE_LEDS 6
   #define NUM_SECOND_LEDS 6
   #define NUM_LEDS (NUM_HOUR_LEDS + NUM_MINUTE_LEDS + NUM_SECOND_LEDS)

   #define LED_TYPE           WS2812B     // Datasheet: http://bit.ly/LED-WS2812B
   #define COLOR_ORDER         GRB        // For color ordering use this sketch: http://bit.ly/RGBCalibrate   

   #define DEFAULT_DEBOUNCE_DELAY    75   // The default debounce delay in milliseconds for the buttons
   #define DEFAULT_BRIGHTNESS        30   // The best tested LEDs brightness 20-60
   #define DEFAULT_ALARM_REPEAT       3   // How many times play the melody alarm
   #define ALARM_1 1                      // Alarm 1 t, available on the RTC DS3231, adds seconds.
   #define ALARM_2 2                      // Alarm 2, the default alarm used by the shield.
   
   #define CA_ON   LOW                    // The value when ON  for CA connections
   #define CC_ON  HIGH                    // The value when ON  for CC connections
   #define CA_OFF HIGH                    // The value when OFF for CA connections
   #define CC_OFF  LOW                    // The value when OFF for CC connections

   // This determines if the menu and/or time are also displayed on the serial monitor.
   // If SERIAL_SETUP_CODE is defined, code to display the serial menu is included in the project.
   // If SERIAL_TIME_CODE  is defined, code to display the serial time, every second, is included in the project.
   // The DEFAULT_SERIAL_SETUP and DEFAULT_SERIAL_TIME values are used to determine if the serial 
   //                 Setup and/or Time messages are displayed initially or not.
   //                 use the public methods 'set_isSerialSetup()' and 'set_isSerialTime()' to
   //                 enable/disable the serial output at runtime. H/W buttons, if defined, 
   //                 can also be used to enable/disable the serial output at runtime.
   #define SERIAL_SETUP_CODE  true        // If (true) - serial setup code included, (false) - code removed
   #define SERIAL_TIME_CODE   true        // If (true) - serial time  code included, (false) - code removed
   #define DEFAULT_SERIAL_SETUP  true     // Initial serial setup display value (allow the setup)
   #define DEFAULT_SERIAL_TIME  false     // Initial serial time display value
   // This controls the inclusion/removal of the code to support hardware buttons/switches to also control the serial output.
   // The serial output can always be controlled in software is the SERIAL_xxxx_CODE is defined (true).
   #define HW_DEBUG_SETUP ((DEBUG_SETUP_PIN >= 0) && (SERIAL_SETUP_CODE))  // Include code to support H/W to control setup display
   #define HW_DEBUG_TIME  ((DEBUG_TIME_PIN  >= 0) && (SERIAL_TIME_CODE))   // Include code to support H/W to control time  display
   // The delay, in ms, is set to a high value when using a momentary button so the button can be release quickly and the use will
   // still see the serial output. When using a switch the delay can be short as the user won't need to keep pressing a button.
   #define DEFAULT_DEBUG_OFF_DELAY 3000 
   #define HARDWARE_DEBUG (HW_DEBUG_SETUP ||  HW_DEBUG_TIME) && (SERIAL_TIME_CODE || SERIAL_SETUP_CODE) 

   /// @brief The structure holds all the Alarm information used by the Binary Clock.
   /// @note  The 'melody' selection has not been implemented, it will always use the internal melody
   /// @author Chris-80 (2025/07)
   typedef struct alarmTime
      {
      int      number;        // The number of the alarm: 1 or 2
      DateTime time;          // The time of the alarm as a DateTime object
      int      melody;        // The melody to play when the alarm is triggered, 0 = internal melody
      int      status;        // Status of the alarm: 0 - inactive, 1 - active
      } AlarmTime;

   /// @brief The structure that holds the state of a button.
   ///        It contains the pin number, current state, last read state, last read time
   ///        the onValue (the value when the button is pressed) for CC or CA connections.
   ///        The isPressed() method returns true if the button is currently pressed, false otherwise.
   /// @author Chris-80 (2025/07)
   typedef struct buttonState
      {
      uint8_t pin;                     // The button pin number: e.g. S1, S2, S3, etc.
      int state;                       // The current state of the button: LOW or HIGH
      int lastRead;                    // The last read state of the button: LOW or HIGH
      unsigned long lastReadTime;      // The last time the button was read
      unsigned long lastDebounceTime;  // The last debounce time in milliseconds
      int onValue;                     // The button value when pressed, HIGH (CC) or LOW (CA)
      bool isPressed() const           // True if the button is currently pressed, false otherwise
         { return (state == onValue); }  
      } ButtonState;

   #define DS3231_CONTROL              0x0E  // Control register address for DS3231
   #define DS3231_STATUS               0x0F  // Status register address for DS3231
   #define DS3231_TEMP_MSB             0x11  // Temperature MSB register address for DS3231
   #define DS3231_TEMP_LSB             0x12  // Temperature LSB register address for DS3231
   #define DS3231_ALARM1               0x07  // Alarm 1 register address for DS3231
   #define DS3231_ALARM2               0x0B  // Alarm 2 register address for DS3231
   #define DS3231_ALARM1_MODE_MASK     0x0F  // Mask for Alarm 1 mode in control register
   #define DS3231_ALARM2_MODE_MASK     0x0C  // Mask for Alarm 2 mode in control register
   #define DS3231_ALARM1_STATUS_MASK   0x01  // Mask for Alarm 1 status in control register
   #define DS3231_ALARM2_STATUS_MASK   0x02  // Mask for Alarm 2 status in control register

   /// @brief The class that extends the RTCLib's RTC_DS3231 class to add raw read/write methods
   /// @note  This class is a hack for now as the RTCLib does not provide a way to read/write raw registers.
   /// @author Chris-80 (2025/07)
   class RTCLibPlusDS3231 : public RTC_DS3231
      {
   public:
      /// @brief Wrapper for the 'RTC_I2C::read_register' method to read a register from the DS3231 RTC.
      /// @details This method reads a single byte from the specified register of the DS3231
      /// @param reg The DS3231 register number to read.
      /// @return The register value that was read.
      uint8_t RawRead(uint8_t reg);

      /// @brief Wrapper for the 'RTC_I2C::write_register' method to write a value to a register in the DS3231 RTC.
      /// @details This method writes a single byte to the specified register of the DS3231
      /// @param reg The DS3231 register number to write to.
      /// @param value The value to write to the register.
      void RawWrite(uint8_t reg, uint8_t value);
      };

   /// @brief The BinaryClock class encapsulates the functionality of the Binary Clock Shield for Arduino.
   ///        It provides all the methods needed to initialize, set the date/time, set the alarm, and 
   ///        change the brightness and LED colors in addition to handling the LED display.
   /// @details The class has public methods to get/set the time, alarm, alarm melody, and brightness.
   ///          It also has methods to handle the settings menu on a serial display. Refactoring the 
   ///          original example code into a class was done to pair it with another class that handles
   ///          the WiFi connection and allows the user to change the LED colors and melodies used for
   ///          the alarm at runtime, without needing to recompile the code. The original code and shield design
   ///          targeted an Arduino UNO board, this is designed for an ESP32 based UNO board, such as 
   ///          the Wemos D1 R32 UNO or the new ESP32-S3 UNO. The idea is to connect to a NTP server over WiFi.
   /// @remarks This class is designed to be used with the 'Binary Clock Shield for Arduino' by 
   ///          Marcin Saj (available from: https://nixietester.com), original source code: 
   ///          https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
   ///          This classes uses the Adafruit RTCLib library for the RTC functionality 
   ///          (https://github.com/adafruit/RTClib) in place of the original DS3232RTC library by 
   ///          Jack Christensen (https://github.com/JChristensen/DS3232RTC) used in the original code.
   /// @author  Chris-80 (2025/07)
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
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      /// @author Chris-80 (2025/07)
      void settingsMenu();

      /// @brief The method called to play the alarm melody.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      void playAlarm();

      //#################################################################################//
      // PROPERTIES
      //#################################################################################//

      /// @brief The method called to set the current 'Time' property.
      /// @param value The DateTime object containing the current time to set.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @author Chris-80 (2025/07)
      void set_Time(DateTime &value);

      /// @brief The method called to get the current 'Time' property.
      /// @return A DateTime object containing the current time.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @author Chris-80 (2025/07)
      DateTime get_Time();

      /// @brief The method called to set the current 'Alarm' property.
      /// @param alarmTime The AlarmTime structure containing the alarm time and status.
      /// @note The AlarmTime structure contains the hour, minute, and status of the alarm
      ///       The status is 0 for inactive, 1 for active.
      ///       Hours are 0 to 23.
      /// @author Chris-80 (2025/07)
      void set_Alarm(AlarmTime &value);

      /// @brief The method called to get the default 'Alarm' property
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @note The returned AlarmTime is overwritten on each call. Make a local copy if needed.
      /// @author Chris-80 (2025/07)
      AlarmTime get_Alarm() { return GetAlarm(ALARM_2); }

      /// @brief The method called to get the 'AlarmTime' for alarm 'number'
      /// @param number The alarm number: 1 or 2. Alarm 2 is the default alarm.
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @note The returned AlarmTime is overwritten on each call. Make a local copy if needed.
      /// @design This method was included as a workaround to allow the user to get alarm 1
      ///         without breaking the property pattern for the Alarm, so no '_' after get....
      /// @author Chris-80 (2025/07)
      AlarmTime GetAlarm(int number);

      /// @brief Property pattern for the 'isSerialSetup' flag property.
      ///        This property controls whether the serial setup menu is displayed or not.
      /// @author Chris-80 (2025/07)
      void set_isSerialSetup(bool value);
      bool get_isSerialSetup() const;

      /// @brief Property pattern for the 'isSerialTime' flag property.
      ///        This property controls whether the serial time is displayed or not.
      /// @author Chris-80 (2025/07)
      void set_isSerialTime(bool value);
      bool get_isSerialTime() const ;

      /// @brief Property pattern for the LED 'Brightness' property.
      ///        This property controls the brightness of the LEDs, 0-255, 20-30 is normal
      /// @author Chris-80 (2025/07)
      void set_Brightness(byte value); 
      byte get_Brightness();

      /// @brief Property pattern for the 'DebugOffDelay' property. This controls how fast 
      ///        the serial time monitor is turned off after the debug pin goes OFF.
      /// @author Chris-80 (2025/07)
      void set_DebugOffDelay(unsigned long value);
      unsigned long get_DebugOffDelay() const;

      /// @brief Method to change the alarm melody with a melody and note duration arrays.
      /// @param melodyArray The array of melody notes to play, each note is a frequency in Hz.
      /// @param melodySize The size (i.e. number of notes) of the melody array.
      /// @param noteDurationArray The array of note durations in milliseconds, each note duration corresponds to a note in the melody.
      /// @param noteDurationSize The size of the note duration array, must match the melodySize.
      /// @return Flag: true - success, false - failure (e.g. if the arrays are null or sizes do not match).
      /// @author Chris-70 (2025/07)
      bool SetAlarmMelody(unsigned *melodyArray, size_t melodySize, unsigned long *noteDurationArray, size_t noteDurationSize)
         {
            bool result = false;
         if (melodyArray != nullptr && noteDurationArray != nullptr && melodySize > 0 && noteDurationSize == melodySize)
            {
            this->melodyAlarm = melodyArray;
            this->melodySize = melodySize;
            this->noteDurations = noteDurationArray;
            this->noteDurationsSize = noteDurationSize;
            result = true;
            }

         return result;
         }

      /// @brief Method to convert a DateTime value to a string inline. This method takes the format as a parameter
      ///        and copies it to the buffer before calling DateTime.toString() and returning the result.
      /// @note  The DateTime.toString(char *buffer) method reads the buffer parameter to extract the 
      ///        formatting string before performing the conversion, placing the result back in the buffer and
      ///        returning a pointer to the same buffer. This forces the developer to litter their code with 
      ///        lines to initialize the buffer with the formatting string. This method changes that, you just 
      ///        pass the format string as a parameter and it is copied to the buffer before calling DateTime.toString().
      /// @param time The DateTime object to convert to a string.
      /// @param buffer The buffer to store the resulting string. Size the buffer to hold the resulting string, the
      ///               DateTime.toString() methods is vulnerable to buffer overrun.
      /// @param size The size of the buffer, not including the null '\0' terminator.
      /// @param format The format string to use for the conversion, default is "YYYY/MM/DD hh:mm:ss".
      /// @author Chris-80 (2025/07)
      char* DateTimeToString(DateTime time, char* buffer, size_t size, const char* format = "YYYY/MM/DD hh:mm:ss") const
         {
         if (buffer == nullptr || size == 0)
            return nullptr; // Return null if buffer is null

         strncpy(buffer, format, size);
         return time.toString(buffer);
         }

      /// @brief Singleton pattern to ensure only one instance of BinaryClock
      /// @author Chris-80 (2025/07)
      static BinaryClock& get_Instance()
         {
         static BinaryClock instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }

   protected:
      /// @brief Default Constructor for the BinaryClock class. This initializes the 
      ///        button states, settings options, and brightness. It assigns the 
      ///        melody and note durations arrays to the class members.
      BinaryClock(); 

      /// @brief Destructor for the BinaryClock class. This destructor is 
      ///        empty as there is no dynamic memory allocation in this class.
      virtual ~BinaryClock();

      // Singleton pattern - Disable these constructors and assignment operators.
      BinaryClock (const BinaryClock&) = delete;            // Disable copy constructor
      BinaryClock& operator=(const BinaryClock&) = delete;  // Disable assignment operator
      BinaryClock (BinaryClock&&) = delete;                 // Disable move constructor
      BinaryClock& operator=(BinaryClock&&) = delete;       // Disable move assignment operator

      /// @brief This method is to isolate the code needed to initialize the Buttons.
      ///        The 'ButtonState.onValue' determines the type: INPUT_PULLUP/DOWN.
      /// @author Chris-70 (2025/07)
      void initializeButtons();

      /// @brief This method is to isolate the code needed to setup for the RTC.
      /// @author Chris-80 (2025/07)
      void setupRTC();

      /// @brief This method is to isolate the code needed to setup the alarm.
      /// @author Chris-80 (2025/07)
      void setupAlarm();

      /// @brief This method is to isolate the code needed to setup the FastLED library.
      /// @author Chris-80 (2025/07)
      void setupFastLED();

      /// @brief Property: 'DebounceDelay' time (ms) for the buttons. Initially set to  DEFAULT_DEBOUNCE_DELAY.
      /// @author Chris-70 (2025/07)
      void set_DebounceDelay(unsigned long value);
      unsigned long get_DebounceDelay() const;

      // ################################################################################
      // ORIGINAL METHODS - 
      // ################################################################################

      /// @brief The method called when the RTC generates an interrupt on the 'RTC_INT' pin every second.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      void RTCinterrupt();

      /// @brief The method called to read the alarm time status (ON/OFF), for the default alarm, from the RTC.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void getAlarmTimeAndStatus();

      /// @brief The method called to write the alarm time status (ON/OFF), for the default alarm, from the RTC.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setAlarmTimeAndStatus();

      /// @brief The method called to read the current time from the RTC and update the LEDs.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void getAndDisplayTime();

      /// @brief The method called to convert the time to binary and update the LEDs.
      /// @param hourRow The value for the top, to display the hour LEDs (16-12).
      /// @param minuteRow The value for the middle, to display the minute LEDs (11-6).
      /// @param secondRow The value for the bottom, to display the second LEDs (5-0).
      /// @details This method converts the current time to binary and updates the LEDs 
      ///          using the color values defined in the arrays 'OnColor' and 'OffColor'
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void convertDecToBinaryAndDisplay(int hourRow, int minuteRow, int secondRow);

      /// @brief The method called to set the time on the RTC from the value in the time field.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setNewTime();

      /// @brief This method is called when the user exceeds the current time element limits.
      ///        The value is rolled over to the next valid value, e.g. 59  -> 0, or 0 -> 59.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setCurrentModifiedValue();

      /// @brief The method called to check the current modified value format to stay within limits,
      ///        Hours 0 - 2; Minutes 0 - 59; Seconds 0 - 59, while the user is changing them.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void checkCurrentModifiedValueFormat();

      /// @brief This method is used to save either the new time or alarm time set by the user.
      /// @details This method is called when the user has set a new time or alarm time from
      ///          the buttons on the Binary Clock Shield.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void saveCurrentModifiedValue();

      /// @brief This method displays the value as the user is changing it. Only one row is
      ///        displayed at a time while the time is being update.: Hours; Minutes; Seconds or Alarm ON/OFF.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void displayCurrentModifiedValue();

      #if SERIAL_TIME_CODE
      /// @brief The method called to display the current time, decimal and binary, over the serial monitor.
      /// @details While this method can still be removed at compile time, it can also be controlled, at run-time, 
      ///          in software and hardware. This method is called every second so being able to control the         
      ///          output in software and hardware, by using a switch or jumper, can start/stop the serial time display.
      /// @remarks This code was modified from the original Binary Clock Shield for Arduino by Marcin Saj.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void serialTime();
      #endif

      // These methods are all part of the serial menu display and can be removed at compile time. They can be 
      // can be controlled in both software and hardware. A momentary button is used to toggle the menu on/off.
      #if SERIAL_SETUP_CODE
      /// @brief The method called to display the serial startup information.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      void serialStartInfo();

      /// @brief The method called to display the serial menu for setting the time and alarm.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      void serialSettings();

      /// @brief The method called to display the alarm settings over the serial monitor.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      void serialAlarmInfo();

      /// @brief The method called to display the current modified value over the serial monitor.
      /// @details This method is called when the user is changing the time or alarm time.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      void serialCurrentModifiedValue();
      #endif

      /// @brief Method to check if the button was pressed ON from OFF since the last call.
      /// @param button - The ButtonState structure containing the button state, pin, type, etc.
      /// @return True if the button is pressed ON from OFF, false otherwise (button OFF or button ON and no change).
      /// @note The method returns false if the button is ON but has not changed state since the last read.
      ///       Check the ButtonState::isPressed() property to see if the button is currently pressed or not. 
      /// @details This method handles buttons that are wired either CC and CA where the concept of ON or PRESSED is
      ///          defined by the way it is wired. As a result, the first time it called on a button, it will
      ///          behave as if it had just transitioned from its opposite state to its current state. This fixes a
      ///          bug with switches (or jumpers) that are wired ON at startup.
      /// @remarks This code was adapted and modified from the methods: checkS1(); checkS2; and checkS3() in the 
      ///          original Binary Clock Shield for Arduino by Marcin Saj.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      bool isButtonOnNew(ButtonState& button);

      #if HARDWARE_DEBUG
      /// @brief This method is called to check the hardware debug buttons/switches and set the serial output level.
      /// @author Chris-80 (2025/07)
      void checkHardwareDebugPin();
      #endif

   public:         
      #ifdef ESP32UNO
      static CRGB OnColor [NUM_LEDS]     PROGMEM; // Colors for the LEDs when ON
      static CRGB OffColor[NUM_LEDS]     PROGMEM; // Colors for the LEDs when OFF
      #endif

   protected:
      RTCLibPlusDS3231 RTC;                 // Create RTC object using Adafruit RTCLib library

      CRGB leds[NUM_LEDS] PROGMEM;          // Array of LED colors to display the current time
      bool binaryArray[NUM_LEDS] PROGMEM;   // Serial Debug: Array for binary representation of time

      // The UNO Compiler doesn't support this C++ style object initialization, move it to the constructor.
      AlarmTime alarm1; // = { .number = ALARM_1, .melody = 0, .status = 0 };  // DS3232 alarm, includes seconds in alarm.
      AlarmTime alarm2; // = { .number = ALARM_2, .melody = 0, .status = 0 };  // Default alarm, seconds set at 00.

      // Note durations: 4 = quarter note, 8 = eighth note, etc.:
      // Some notes durations have been changed (1, 3, 6) to make them sound better
      static const unsigned long NoteDurations[] PROGMEM; // Note durations array, unsigned long array (64 bits)
      static const unsigned      MelodyAlarm[] PROGMEM;   // Melody for alarm, unsigned integer array (32 bits)
      static const size_t        MelodySize; // Size of the melody array
      static const size_t        NoteDurationsSize; // Size of the note durations array

      // These variables are initially set to the internal static melody and note durations arrays
      // They can be changed to use different melodies and note durations in the ESP32 flash memory.
      unsigned      *melodyAlarm;         // Pointer to the melody array
      int            melodySize;          // Size of the melody array
      unsigned long *noteDurations;       // Pointer to the note durations array
      int            noteDurationsSize;   // Size of the note durations array

   private:
      // The 3 buttons used to control the Binary Clock Shield menu for setting the time and alarm.
      ButtonState buttonS1          = { .pin = S1,              .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = CC_ON };
      ButtonState buttonS2          = { .pin = S2,              .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = CC_ON };
      ButtonState buttonS3          = { .pin = S3,              .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = CC_ON };
      #if HARDWARE_DEBUG && HW_DEBUG_SETUP
      ButtonState buttonDebugSetup  = { .pin = DEBUG_SETUP_PIN, .state = CC_OFF, .lastRead = CC_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = CC_ON };
      #endif
      #if HARDWARE_DEBUG && HW_DEBUG_TIME
      ButtonState buttonDebugTime   = { .pin = DEBUG_TIME_PIN,  .state = CA_OFF, .lastRead = CA_OFF, .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = CA_ON };
      #endif

      DateTime time;                      // Current time from the RTC
      DateTime tempTime;                  // Temporary time variable
      int countButtonPressed;             // Counter for button pressed
      static volatile bool RTCinterruptWasCalled;   // Flag for RTC interrupt was called

      unsigned long debounceDelay = DEFAULT_DEBOUNCE_DELAY; // The debounce time for a button press.

      // Variables that store the current settings option
      int settingsOption = 0;               // Time = 1, Alarm = 3  
      int settingsLevel = 0;                // Hours = 1, Minutes = 2, Seconds / On/Off Alarm = 3

      int alarmRepeatMax = DEFAULT_ALARM_REPEAT;   // Maximum alarm repeat count
      int alarmRepeatCount = 0;                    // Current alarm repeat count
      byte brightness = DEFAULT_BRIGHTNESS;        // Brightness of the LEDs, 0-255

      char buffer[64] = { 0 };                     // Buffer for the DateTime string conversion

      bool isSerialSetup = (SERIAL_SETUP_CODE) && (DEFAULT_SERIAL_SETUP); // Serial setup flag
      bool isSerialTime  = (SERIAL_TIME_CODE)  && (DEFAULT_SERIAL_TIME);  // Serial time flag   

      // Time to wait after serial time button goes off before stopping the serial output.
      // Set to a long delay if using a momentary button, keep short for a switch. This
      // allows a button to be pressed, released and you still get output for 'debugDelay' ms.
      unsigned long debugDelay = DEFAULT_DEBUG_OFF_DELAY; 
      };
   }
#endif