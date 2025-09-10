/// @file BinaryClock.h
/// @brief Software for the fantastic Binary Binary Clock Shield for Arduino 
/// UNO, designed and maufactured by Marcin Saj https://nixietester.com  available at:
/// https://nixietester.com/product/binary-clock-shield-for-arduino/
/// @details
/// This software was losely based on Example 11 from:  
/// https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino  
/// The original file comments are reproduced here.
/// --------  
/// Binary Clock RTC 24H with Interrupt, Alarm and Buttons Example
/// This example demonstrates complete Binary Clock with Time and Alarm settings
///   
/// It is recommended that the first start should be carried out with the serial terminal, 
/// for better knowing the setting options. 
///   
/// The buttons allows you to set the time and alarm - exact hour, minute, second/alarm status.
/// Alarm causes melody to play.  
/// How to use piezo with the tone() command to generate notes you can find here:
/// http://www.arduino.cc/en/Tutorial/Tone  
///     
/// A falling edge at the RTC INT/SQW output causes an interrupt, 
/// which it uses for regular - 1 per second - reading time from RTC and 
/// checking alarm status flag 'A2F'. Since we use RTC INT/SQW output for
/// regular reading current time - square wave output SQW option, 
/// global interrupt flag INTCN is set to 0, this disables the interrupts from both RTC alarms.
/// Referring to the documentation: when the INTCN is set to logic 0, 
/// the 'A2F' bit does not initiate an interrupt signal. By turning off the interrupts from the alarms, 
/// we can use the interrupt flag 'A2IE' as an info flag whether the alarm has been activated or not. 
/// Check RTC datasheet page 11-13 http://bit.ly/DS3231-RTC  
///   
/// @verbatim 
/// Hardware:
/// 1) Arduino Uno style board. e.g. 
///    - Arduino UNO R3                    (https://store.arduino.cc/products/arduino-uno-rev3)
///    - Arduino UNO R4 Minima             (https://store.arduino.cc/products/uno-r4-minima)
///    - Arduino UNO R4 WiFi               (https://store.arduino.cc/products/uno-r4-wifi)
///    - Adafruit Metro ESP32-S3           (https://www.adafruit.com/product/5500)
///    - Generic Wemos ESP32 D1 R32 UNO (*)
///    - Generic ESP32-S3 UNO              (uses the pinout of an ESP32-S3 DevKitC-1)
/// 2) Binary Clock Shield for Arduino     (https://nixietester.com/product/binary-clock-shield-for-arduino/)
/// 3) Battery CR1216/CR1220 
/// 
/// INT/SQW   connected to: Arduino pin   3 INT1 ; Metro ESP32-S3   3 ; ESP32_D1-R32 25  ; ESP32-S3 UNO 17
/// PIEZO     connected to: Arduino pin  11  PWM ; Metro ESP32-S3  11 ; ESP32_D1-R32 23  ; ESP32-S3 UNO 11
/// S3 button connected to: Arduino pin  A0      ; Metro ESP32-S3  A0 ; ESP32_D1-R32  2  ; ESP32-S3 UNO  2
/// S2 button connected to: Arduino pin  A1      ; Metro ESP32-S3  A1 ; ESP32_D1-R32  4  ; ESP32-S3 UNO  1
/// S1 button connected to: Arduino pin  A2      ; Metro ESP32-S3  A2 ; ESP32_D1-R32 35  ; ESP32-S3 UNO  7
/// LEDs      connected to: Arduino pin  A3      ; Metro ESP32-S3  A3 ; ESP32_D1-R32 15* ; ESP32-S3 UNO  6
/// RTC SDA** connected to: Arduino pin PC4  SDA ; Metro ESP32-S3  47 ; ESP32_D1-R32 36  ; ESP32-S3 UNO  8
/// RTC SCL** connected to: Arduino pin PC5  SCL ; Metro ESP32-S3  48 ; ESP32_D1-R32 39  ; ESP32-S3 UNO  9
///
///  * (Requires hardware board modification to use LED output pin. Wire pin 15 on the D1-R32 board to
///    the SHIELD pin that connects to A3/34. This is where the LED data pin is connected to.
///    See the README.md file: https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md)
/// ** (The shield is connected to and uses the alternate SDA (A4) and SCL (A5) pins: PC4; and PC5.
///    These pins are located just below the RESET button and above the AREF pin)
///
///                        +------+       +------+       +------+       +------+       +------+
///                        |LED 16|---<---|LED 15|---<---|LED 14|---<---|LED 13|---<---|LED 12|--<-+
///                        +------+       +------+       +------+       +------+       +------+    |
///                                                                                                |
///    +--------------->-------------->-------------->-------------->-------------->---------------+
///    |
///    |    +------+       +------+       +------+       +------+       +------+       +------+
///    +----|LED 11|---<---|LED 10|---<---|LED 09|---<---|LED 08|---<---|LED 07|---<---|LED 06|--<-+
///         +------+       +------+       +------+       +------+       +------+       +------+    |
///                                                                                                |
///    +--------------->-------------->-------------->-------------->-------------->---------------+
///    |
///    |    +------+       +------+       +------+       +------+       +------+       +------+
///    +----|LED 05|---<---|LED 04|---<---|LED 03|---<---|LED 02|---<---|LED 01|---<---|LED 0 |--<-- DATA_PIN 
///         +------+       +------+       +------+       +------+       +------+       +------+       (A3)
/// 
/// @endverbatim
/// Note: (Chris-80 2025/07)
///     
/// This file has been adapted from the original Example; "11-BinaryClock-24H-RTCInterruptAlarmButtons.ino" file as published
/// on the Binary Clock Shield for Arduino GitHub repository: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
/// The original file was modified to be encapsulated in a class, BinaryClock. The class encapsulates all the functionality
/// of the "Binary Clock Shield for Arduino by Marcin Saj." Modifications were made to support the ESP32 UNO platform and to
/// allow greater flexibility by the user at runtime, such as the color selection and melodies used for the alarm.
///       
/// The goal of using an ESP32 based UNO board was to allow the RTC to be connected to a NTP server over WiFi. The code
/// for the WiFi connection is encapsulated in its own class, 'BinaryClock_NTP', which is not included in this file. 
/// It uses WPS to connect to a WiFi network and stores the credentials in the ESP32 flash memory so future connections are
/// made automatically without user intervention. The WiFi connection also allows the user to change the LED colors
/// and melodies used for the alarm at runtime, without needing to recompile the code.  
///    

#pragma once
#ifndef __BINARYCLOCK_H__
#define __BINARYCLOCK_H__

#include "BinaryClock.Defines.h" /// Include the Binary Clock definitions for the boards, and any hard constants..

#include <Arduino.h>             /// Arduino core library

#include <FastLED.h>             /// https://github.com/FastLED/FastLED
#include "fl/namespace.h"
#include <fl/array.h>            /// For fl::array
#include "fl/namespace.h"

#include <RTClib.h>              /// Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>           /// https://github.com/janelia-arduino/Streaming

#include "pitches.h"             /// Needed to create the pitches. Library: https://arduino.cc/en/Tutorial/ToneMelody

#if STL_USED
   #include <stdint> 
   #include <vector>             /// STL std::vector for the alarm melodies.
#endif

#ifdef TESTING                   ///< Changes needed for unit testing of this code.
   #define TEST_VIRTUAL virtual        ///< Virtul methods for unit testing ony.
   #define TEST_PROTECTED protected:   ///< Access specifier for unit testing ony.
#else
   #define TEST_VIRTUAL                ///< Virtual methods for unit testing, removed otherwise.
   #define TEST_PROTECTED              ///< Access specifier for unit testing, removed otherwise.
#endif

// using namespace fl;

// typedef unsigned char uint8_t;   ///< Unsigned 8 bit

/// @namespace BinaryClockShield
/// @brief
/// Namespace for all Binary Clock software used in this project.
/// Protect from any possible name clashes by putting all code in its own namespace.
namespace BinaryClockShield
   {
   /// @brief The structure holds all the Alarm information used by the Binary Clock.
   /// @details This structure contains all the information related to a specific alarm, including
   ///          the alarm number, time, melody, status, and whether it has fired or not.   
   ///          While repeating the alarm based the date or day of the week instead of just daily is
   ///          supported by the DS3231 RTC, ... @todo finish.
   /// @note  The 'melody' selection has not been implemented, it will always use the internal melody
   /// @author Chris-80 (2025/07)
   typedef struct alarmTime
      {
      enum Repeat             ///< Alarm repeation when ON. Default is Daily.
         {
         Never,               ///< The alarm is turned OFF after it has fired.
         Hourly,              ///< The alarm repeats every hour at the selected minute
         Daily,               ///< The alarm repeats every day at the same time. This is the default.
         Weekly,              ///< The alarm repeats every week on the given day and time.
         Monthly,             ///< The alarm repeats every month on the given date and time.
         endTag               ///< Always the last enum value, defines the size.
         };
      uint8_t  number;        ///< The number of the alarm: 1 or 2
      DateTime time;          ///< The time of the alarm as a DateTime object
      uint8_t  melody;        ///< The melody to play when the alarm is triggered, 0 = internal melody
      uint8_t  status;        ///< Status of the alarm: 0 - inactive, 1 - active
      Repeat   freq;          ///< The alarm repeat frequency, default: Daily.
      bool     fired;         ///< The alarm has fired (e.g. alarm is 'ringing').
      void clear()            ///< Clear all data except the alarm 'number'
         {
         time = DateTime();   ///< 00:00:00 (2000-01-01)
         melody = 0;          ///< Default melody number, internal
         status = 0;          ///< OFF, alarm is not set.
         freq   = Daily;      ///< The alarm repeats every day when ON.
         fired  = false;      ///< Alarm is not ringing (OFF)
         }
      } AlarmTime;

   /// @brief The structure that holds the state of a button.
   /// @details It contains the pin number, current state, last read state, last read time
   ///          the onValue (the value when the button is pressed) for CC (HIGH) or CA (LOW) connections.
   ///          The isPressed() method returns true if the button state is ON (pressed), false otherwise.
   ///          The read() method reads the current button pin state without debouncing. True: ON, False: OFF.
   ///          The value() method reads the current raw pin value without debouncing, returns HIGH/LOW.
   /// @author Chris-80 (2025/07)
   typedef struct buttonState
      {
      uint8_t pin;                     ///< The button pin number: e.g. S1; S2; S3, etc. (defined in 'BinaryClock.Defines.h')
      uint8_t state;                   ///< The current state of the button: LOW or HIGH
      uint8_t lastRead;                ///< The last read state of the button: LOW or HIGH
      unsigned long lastReadTime;      ///< The last time the button was read
      unsigned long lastDebounceTime;  ///< The last debounce time in milliseconds
      uint8_t onValue;                 ///< The button value when pressed, HIGH (CC) or LOW (CA)
      TEST_VIRTUAL bool isPressed(int value) const  ///< True if the 'value' is currently ON (pressed), false otherwise
         { return (value == onValue); }  
      TEST_VIRTUAL bool isPressed() const ///< True if the button 'state' is currently 'ON' (pressed), false otherwise
         { return isPressed(state); }  
      TEST_VIRTUAL bool read()            ///< Read and the current button H/W state: ON/OFF (no debounce done).
         { return isPressed(value()); }
      TEST_VIRTUAL int value()            ///< Read the current raw pin value (no debounce), return HIGH/LOW.
         { return digitalRead(pin); }
      } ButtonState;
   
   /// @brief The structure to create a note with a sound frequency and duration.
   /// @details The melody used to signal an alarm uses an array of these to create
   ///          the alarm sound.
   ///          See: 
   ///          Create melodies like this:
   /// @verbatim
   ///          Note melody[] = {
   ///              { 262, 500 },  // C4
   ///              { 294, 500 },  // D4
   ///              { 330, 500 },  // E4
   ///              { 349, 500 },  // F4
   ///              { 392, 500 },  // G4
   ///              { 440, 500 },  // A4
   ///              { 494, 500 },  // B4
   ///              { 523, 500 }   // C5
   ///          };
   /// @endverbatim
   struct Note
      {
      unsigned tone;          ///< The tone frequency in Hz.
      unsigned long duration; ///< The duration of the tone in ms.
      // /// @brief Constructor to initialize a Note object.
      // /// @param t The tone frequency in Hz.
      // /// @param d The duration of the tone in ms.
      // Note(unsigned t, unsigned long d) : tone(t), duration(d) {}
      // Note(unsigned long d) : tone(0), duration(d) {} ///< Constructor to initialize a Note object with duration only.
      // Note() : tone(0), duration(0) {}  ///< Default constructor, tone=0, duration=0
      };

   /// @brief The class that extends the RTCLib's RTC_DS3231 class to add raw read/write methods
   /// @remarks This class is a hack for now as the RTCLib does not provide a direct way to read/write 
   ///          raw registers. The base RTC_I2C class only has protected methods to read/write registers,
   /// @author Chris-80 (2025/07)
   class RTCLibPlusDS3231 : public RTC_DS3231
      {
   public:
      /// @brief Wrapper for the 'RTC_I2C::read_register' public method to read a register from the DS3231 RTC.
      /// @details This method reads a single byte from the specified register of the DS3231
      /// @param reg The DS3231 register number to read.
      /// @return The register value that was read.
      uint8_t RawRead(uint8_t reg);

      /// @brief Wrapper for the 'RTC_I2C::write_register' public method to write a value to a register in the DS3231 RTC.
      /// @details This method writes a single byte to the specified register of the DS3231
      /// @param reg The DS3231 register number to write to.
      /// @param value The value to write to the register.
      void RawWrite(uint8_t reg, uint8_t value);
      };

   /// @brief The BinaryClock class encapsulates the functionality of the Binary Clock Shield for Arduino.
   ///        It provides all the methods needed to initialize, set the date/time, set the alarm, and 
   ///        change the brightness and LED colors in addition to handling the LED display. This now
   ///        supports display in 12 hour mode and the settings UX has been improved.
   /// @details The class has public methods to get/set the time, alarm, alarm melody, and brightness.
   ///          It also has methods to handle the settings menu on a serial display. Refactoring the 
   ///          original example code into a class was done to pair it with another class that handles
   ///          the WiFi connection and allows the user to change the LED colors and melodies used for
   ///          the alarm at runtime, without needing to recompile the code. The original code and shield design
   ///          targeted an Arduino UNO board, this is designed for an ESP32 based UNO board, such as 
   ///          Adafruit Metro ESP32 S3 (https://www.adafruit.com/product/5500), Arduino UNO R4 Minima (no WiFi)
   ///          (https://store.arduino.cc/products/uno-r4-minima), Arduino UNO R4 WiFi (which has an ESP32-S3)
   ///          (https://store.arduino.cc/products/uno-r4-wifi), the Wemos D1 R32 UNO or the new ESP32-S3 UNO. 
   ///          The idea is to connect to a NTP server over WiFi.
   /// @remarks This class is designed to be used with the excellent 'Binary Clock Shield for Arduino' by 
   ///          Marcin Saj (available from: https://nixietester.com/product/binary-clock-shield-for-arduino/), 
   ///          original source code: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino  
   ///          This class uses the Adafruit RTCLib library for the RTC functionality 
   ///          (https://github.com/adafruit/RTClib) in place of the original DS3232RTC library by 
   ///          Jack Christensen (https://github.com/JChristensen/DS3232RTC) used in the original code.  
   ///          One big reason for this is the inclusion of the classes 'DateTime' and 'TimeSpan'
   ///          which closely resemble the C# classes. The implementation of DateTime keeps the
   ///          time in individual bytes for Year; Month; Day; Hours; Minutes; and Seconds which is
   ///          close to the format that the RTC uses. This class has been fully documented.
   /// @author  Chris-80 (2025/07)
   class BinaryClock
      {
   public:
      /// @brief The method called to initialize the Binary Clock Shield.
      ///        This has the same functionality of the Arduino setup() method.
      ///        Call this method before using the BinaryClock class.
      /// @param testLeds Flag - Show the test patterns at startup.
      void setup(bool testLeds);
      void setup() { setup(false); };

      /// @brief The method called to run the Binary Clock.
      ///        This has the same functionality of the Arduino loop() method.
      ///        Call this method in the main loop of your program or run this
      ///        method in a separate thread that just loops forever.
      void loop();

      //################################################################################//
      // SETTINGS
      //################################################################################//
      /*!
       @brief The method called to set the time and/or alarm from the shield
                The S1 button sets the Time, S3 sets the Alarm, S2 accepts
                the current modified value and moves to the next line.
                The S3 and S1 buttons increment/decrement the current modified value.
       @details The 'settingsMenu()' method displays the settings menu on the shield LEDs.
                The user can navigate through the menu using the S1, S2, and S3 buttons.
       @verbatim
                To enter the Alarm settings, the user presses the S3 button.
                To enter the Time  settings, the user presses the S1 button.
                The first selection (i.e. Level 1) is for the state or mode:
                    Alarm: ON; OFF; or Abort the alarm setting.
                    Time: 12 Hr; 24 Hr; or Abort the time setting.
                The second selection (i.e. Level 2) is for the hour.
                The third  selection (i.e. Level 3) is for the minute.
                The fourth selection (i.e. Level 4) is for the second (Time only)
       @endverbatim
                When the final selection is made the 'Rainbow' pattern is displayed
                to indicate to the user the changes are over and the settings are 
                either being saved, indicated by the Green checkmark [✅], or the 
                changes have been discarded, indicated by the Pink 'X' [❌] on the shield.
      @verbatim
      
                             +-------------------------------+
                             |           SETTINGS            |
                 +-----------+-------------------------------+
                 |  BUTTONS  |    S3   |     S2    |   S1    |
       +---------+-----------+---------+-----------+---------+
       |         |           |   SET   |   ALARM   |  SET    |
       |         | Level = 0 |  ALARM  |   MELODY  |  TIME   |
       |   S     |           |         |   STOP    |         |
       +   E L   +-----------+---------+-----------+---------+
       |   T E   | Level = 1 |    +    |   SAVE    |    -    |
       |   T V   |           |         | LEVEL = 2 |         |
       +   I E   +-----------+---------+-----------+---------+
       |   N L   | Level = 2 |    +    |   SAVE    |    -    |
       |   G     |           |         | LEVEL = 3 |         |
       +   S     +-----------+---------+-----------+---------+
       |         | Level = 3 |    +    |   SAVE    |    -    |
       |         |           |         | LEVEL 0/4 |         |
       +         +-----------+---------+-----------+---------+
       |         | Level = 4 |    +    |   SAVE    |    -    |
       |         |           |         | LEVEL = 0 |         |
       +---------+-----------+---------+-----------+---------+
      
       @endverbatim
       When setting Time, the first option is to select the 
       Display mode: 12 Hr; 24 Hr; or Abort time setting.
       When setting Alarm, the first option is to select the
       Alarm state: ON; OFF; or Abort the alarm setting.
       @verbatim
       
                             +-------------------------------+
                             |        SETTINGS OPTION        |
                             +---------------+---------------+
                             |   ALARM = 3   |   TIME = 1    |
       +---------------------+---------------+---------------+
       |         | Level = 1 |     3/1       | 1/1   Abort / |  
       |         |           | On/Off/Abort  | 12 Hr/ 24 Hr  |  
       |   S     |           |  (Row: All)   |   (Row: H)    |
       +   E L   +-----------+---------------+---------------+
       |   T E   | Level = 2 |     3/2       |     1/2       |
       |   T V   |           |     HOUR      |     HOUR      |
       |   I E   |           |   (Row: H)    |   (Row: H)    |
       +   N L   +-----------+---------------+---------------+
       |   G     | Level = 3 |     3/3       |     1/3       |
       |   S     |           |    MINUTE     |    MINUTE     |
       |         |           |   (Row: M)    |   (Row: M)    |
       +         +-----------+---------------+---------------+
       |         | Level = 4 |     N/A       |     1/4       |
       |         |           |               |    SECOND     |
       |         |           |               |   (Row: S)    |
       +---------+-----------+---------------+---------------+
      
      @endverbatim
      @author Marcin Saj - From the original Binary Clock Shield for Arduino
      @author Chris-80 (2025/07)
      */
      void settingsMenu();

      /// @brief The method called to play the alarm melody.
      /// @details This method is only used for the Arduino UNO R3.
      ///          Boards that fully support the STL library play the melody
      ///          using a vector of Note structures registered for the alarm.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      /// @author Chris-70 (2025/09)
      void playAlarm();

      //#################################################################################//  
      // Public PROPERTIES   
      //#################################################################################//   

      /// @defgroup properties Properties
      /// @{
      /// @brief Property pattern compatible with all .net languages that have properties (e.g. C#).  
      /// A property such as: @c type @c PropertyName
      /// is implied from C++ getters and setters that follow the pattern:  
      /// - type get_Propertyname()   
      /// - void set_Propertyname(type value)   
      /// @details
      /// In languages, such as C#, where properties are part of the language, these
      /// property patterns are used to translate between properties and the getter/setter
      /// methods from languages that don't support properties, such as C++:   
      ///   
      /// example: C# to/from C++
      /// ========
      /// C# property
      /// @code{.cs}
      ///    String SomePropertyName { get; set; }
      /// @endcode
      /// 
      /// **Where:**   
      ///    <tt> String           : the the data type of the property.</tt>  
      ///    <tt> SomePropertyname : the name of the property.</tt>  
      ///    <tt> get;             : An empty (default) get. Could also be: @c get { your code here; } </tt>   
      ///    <tt> set;             : An empty (default) set. Could also be: @c set { your code here; } </tt>  
      ///    <tt>                    In your code, `value` is the _property value_ passed to set e.g.: myVar = value;</tt>
      /// 
      /// C++ property equivalent: (using setter (@c set_) and getter (@c get_) methods)
      /// @code{.cpp}
      ///   String get_SomePropertyName();
      ///   void   set_SomePropertyName(String value);
      /// @endcode
      /// **Where:**  
      ///    <tt> String           : the the data type of the property. </tt>  
      ///    <tt> SomePropertyname : the name of the property. </tt>  
      ///    <tt> value            : the value to set the property to. The parameter name is always `value` </tt>  
      /// @code{.cpp}
      ///   // e.g. C++ for property definition: DateTime Time;
      ///   DateTime get_Time();
      ///   void     set_Time(DateTime value);
      ///  
      ///   // In langusges such as C# you would be able to write:
      ///   DateTime myTime = Time; // Instead of myTime = get_Time();
      ///   Time = myTime;          // Instead of set_Time(myTime);
      /// @endcode
      /// The compiler translates properties to @c set_ and @c get_ methods.
      /// The @c get_ and @c set_ methods get translated as properties.   
      /// The property pattern is used to identify the properties. 
      /// - Read only properties don't have a @c set_ method.
      /// - Write only properties don't have a @c get_ method.
      ///   
      /// We are following this property pattern for all our "properties" in C++.
      /// If you aren't creating a property, don't use @c "get_"...()  or @c "set_"...()
      /// in your method/function names.
      /// @}

      /// @ingroup properties
      /// @brief Singleton pattern to ensure only one instance of BinaryClock.
      /// @details Call this read only property to get a reference to the instance.
      /// @author Chris-80 (2025/07)
      static BinaryClock& get_Instance()
         {
         static BinaryClock instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }

      /// @ingroup properties
      /// @{
      /// @brief The property methods called to set/get the current 'Time' property.
      /// @param value The DateTime object containing the current time to set.
      /// @return A DateTime object containing the current time.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @see get_Time()
      /// @see ReadTime()
      /// @author Chris-80 (2025/07)
      void set_Time(DateTime value);
      DateTime get_Time() const;
      /// @}

      /// @ingroup properties
      /// @{
      /// @brief The property method called to set/get the current 'Alarm' property.
      /// @param value The AlarmTime structure containing the alarm time and status.
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @note The AlarmTime structure contains the hour, minute, and status of the alarm
      ///       The status is 0 for inactive, 1 for active.
      ///       Hours are 0 to 23.
      /// @see get_Alarm()
      /// @see GetAlarm(int)
      /// @author Chris-80 (2025/07)
      void set_Alarm(AlarmTime value);
      /// @copydoc set_Alarm()
      /// @see set_Alarm()
      /// @sideeffect This reads from the RTC and updates the local alarm field value.
      AlarmTime get_Alarm() { return GetAlarm(ALARM_2); }
      /// @}

      /// @ingroup properties
      /// @brief Property pattern for the LED 'Brightness' property.
      ///        This property controls the brightness of the LEDs, 0-255, 20-30 is normal
      /// @param value The brightness level to set (0-255).
      /// @return The current brightness level (0-255).
      /// @see get_Brightness()
      /// @author Chris-80 (2025/07)
      void set_Brightness(byte value);
      /// @ingroup properties
      /// @copydoc set_Brightness()
      /// @see set_Brightness()
      byte get_Brightness();

      /// @brief Property pattern for the 'is12HourFormat' flag property.
      ///        This property controls whether the time is displayed in 
      ///        12-hour or 24-hour format.
      /// @param value The flag to set (true for 12-hour format, false for 24-hour format).
      /// @return The current flag value (true for 12-hour format, false for 24-hour format).
      /// @see get_Is12HourFormat()
      /// @author Chris-70 (2025-07)
      /// @ingroup properties
      void set_Is12HourFormat(bool value);
      /// @copydoc set_Is12HourFormat()
      /// @see set_Is12HourFormat()
      bool get_Is12HourFormat() const;

      /// @brief Property pattern for the 'isSerialSetup' flag property. 
      ///        This property controls whether the serial setup menu is displayed or not.
      /// @param value The flag to set (true to display the serial setup menu, false to disable it).
      /// @return The current flag value (true to display the serial setup menu, false to disable it).
      /// @see get_IsSerialSetup()
      /// @author Chris-80 (2025/07)
      /// @ingroup properties
      void set_IsSerialSetup(bool value);
      /// @copydoc set_IsSerialSetup()
      /// @see set_IsSerialSetup()
      bool get_IsSerialSetup() const;

      /// @brief Property pattern for the 'IsSerialTime' flag property.
      ///        This property controls whether the serial time is displayed or not.
      /// @param value The flag to set (true to display the serial time, false to disable it).
      /// @return The current flag value (true to display the serial time, false to disable it).
      /// @see get_IsSerialTime()
      /// @author Chris-80 (2025/07)
      /// @ingroup properties
      void set_IsSerialTime(bool value);
      /// @copydoc set_IsSerialTime()
      /// @see set_IsSerialTime()
      bool get_IsSerialTime() const;

      /// @brief Property pattern for the 'OnColors' property.
      ///        This property controls the colors of the LEDs when they are on.
      /// @param value A reference the array of colors to set for the LEDs when they are on.
      /// @return A const reference to the array of colors for the LEDs when they are on.
      /// @see get_OnColors()
      /// @author Chris-70 (2025/08)
      /// @ingroup properties
      void set_OnColors(const fl::array<CRGB, NUM_LEDS>& value);
      /// @copydoc set_OnColors()
      /// @see set_OnColors()
      const fl::array<CRGB, NUM_LEDS>& get_OnColors() const;

      /// @brief Property pattern for the 'OffColors' property.
      ///        This property controls the colors of the LEDs when they are off.
      /// @param value A reference the array of colors to set for the LEDs when they are off.
      /// @return A const reference to the array of colors for the LEDs when they are off.
      /// @note Normally this color is CRGB::Black (i.e. LED is OFF). Any other color will   
      ///       keep the LED on at all times, always consuming power.
      /// @see get_OffColors()
      /// @author Chris-70 (2025/08)
      /// @ingroup properties
      void set_OffColors(const fl::array<CRGB, NUM_LEDS>& value);
      /// @copydoc set_OffColors()
      /// @see set_OffColors()
      const fl::array<CRGB, NUM_LEDS>& get_OffColors() const;

      /// @property OnHour
      /// @brief Property pattern for the 'OnHour' property.
      ///        This property controls the colors of the LEDs when they are ON for the hour display.
      /// @details These values are always used for the HOUR LEDs except when `AmColor` is CRGB::Black 
      ///          AND `Is12HourFormat` is true.
      /// @param value A reference the array of colors to set for the LEDs when they are on for the hour display.
      /// @return A const reference to the array of colors for the LEDs when they are on for the hour display.
      /// @see get_OnHour()
      /// @see set_OnHourAM()
      /// @see get_OnHourAM()
      /// @author Chris-70 (2025/08)
      /// @ingroup properties
      /// @{
      void set_OnHour(const fl::array<CRGB, NUM_HOUR_LEDS>& value);
      const fl::array<CRGB, NUM_HOUR_LEDS>& get_OnHour() const;
      /// @}

      /// @brief Property pattern for the `OnHourAM` property.
      ///        This property controls the colors of the LEDs when they are on for the hour display in AM mode.
      /// @details These color are ONLY used when the `AmColor` is CRGB::Black AND `Is12HourFormat` is true.
      ///          This is to be able to distinguish between 12 midnight in 12 hour mode and 12 noon in 24 hour mode.
      /// @param value A reference the array of colors to set for the LEDs when they are on for the hour display in AM mode.
      /// @return A const reference to the array of colors for the LEDs when they are on for the hour display in AM mode.
      /// @see set_OnHour()
      /// @see get_OnHour()
      /// @see get_OnHourAM()
      /// @author Chris-70 (2025/08)
      /// @ingroup properties
      void set_OnHourAM(const fl::array<CRGB, NUM_HOUR_LEDS>& value);
      /// @copydoc set_OnHourAM()
      /// @see set_OnHourAM()
      const fl::array<CRGB, NUM_HOUR_LEDS>& get_OnHourAM() const;

      /// @brief Property pattern for the 'AmColor' property, used when @see Is12HourFormat is true.
      ///        This property controls the color of the AM indicator LED in 12 hour mode.
      /// @param value The color to set for the AM indicator LED when in 12 hour mode.
      /// @remark This default value is not Black to indicate 12 hour mode. Default is CRGB::DeepSkyBlue
      /// @sideeffect The @see OnColor values for the Hour Row  change when moving to/from Black.
      ///        If AmColor is set to Black, the OnHourAM colors are copied to the Hour Row 
      ///        for the AM hours and @see OnHour colors are copied to OnColors hours for the PM.
      ///        This is to distinguish between 12 midnight in 12 hour mode and 12 noon in 24 hour mode.
      /// @return The current color of the AM indicator LED when in 12 hour mode.
      /// @see get_AmColor()
      /// @author Chris-70 (2025/08)
      /// @ingroup properties
      void set_AmColor(CRGB value);
      /// @copydoc set_AmColor()
      /// @see set_AmColor()
      CRGB get_AmColor() const;

      /// @brief Property pattern for the 'PmColor' property, used when @see Is12HourFormat is true.
      ///        This property controls the color of the PM indicator LED in 12 hour mode.
      /// @param value The color to set for the PM indicator LED when in 12 hour mode.
      /// @return The current color of the PM indicator LED when in 12 hour mode.
      /// @remark This value is normally not Black. Default is CRGB::Indigo
      /// @see get_PmColor()
      /// @author Chris-70 (2025/08)
      /// @ingroup properties
      void set_PmColor(CRGB value);
      /// @copydoc set_PmColor()
      /// @see set_PmColor()
      CRGB get_PmColor() const;

      /// @brief Property: 'DebounceDelay' time (ms) for the button press to stabilize. 
      ///        Initially set to  DEFAULT_DEBOUNCE_DELAY.
      /// @param value The debounce delay time in milliseconds.
      /// @return The current debounce delay time in milliseconds.
      /// @see get_DebounceDelay()
      /// @author Chris-70 (2025/07)
      /// @ingroup properties
      void set_DebounceDelay(unsigned long value);
      /// @copydoc set_DebounceDelay()
      /// @see set_DebounceDelay()
      unsigned long get_DebounceDelay() const;
      
      #if STL_USED
      /// @ingroup properties
      /// @brief Property pattern for the 'MelodyNumber' property using id number to melodyRegistry.
      /// @details This property allows setting and getting the current melody by its registry number.
      ///          Number 0 is always the default melody created from PROGMEM arrays.
      /// @param value The id number of a registered melody to set as the current melody.
      /// @return The current melody registry id number.
      /// @note The melody id number must be valid (number returned from RegisterMelody()) or 
      ///       the setter will ignore it.
      /// @see get_Melody()
      /// @see RegisterMelody()
      /// @see get_CurrentMelody()
      /// @author Chris-70 (2025/09)
      void set_Melody(size_t value);
      /// @copydoc set_Melody()
      /// @see set_Melody()
      size_t get_Melody() const;
   
      /// @ingroup properties
      /// @brief  Read only property: Get the current melody vector by reference.
      /// @return A const reference to the current melody vector.
      /// @see get_Melody()
      /// @author Chris-70 (2025/09)
      const std::vector<Note>& get_CurrentMelody() const;
   
      /// @ingroup properties
      /// @brief Read only property: Get the number of registered melodies.
      /// @return The number of melodies in the registry.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      size_t get_MelodyCount() const { return melodyRegistry.size(); }
      #endif

      #if HW_DEBUG_TIME
      /// @brief Property pattern for the 'DebugOffDelay' property. This controls how fast 
      ///        the serial time monitor is turned off after the debug pin goes OFF.
      /// @see get_DebugOffDelay()
      /// @author Chris-80 (2025/07)
      /// @ingroup properties
      void set_DebugOffDelay(unsigned long value);
      /// @copydoc set_DebugOffDelay()
      /// @see set_DebugOffDelay()
      unsigned long get_DebugOffDelay() const;
      #endif

      //#################################################################################//
      // Public METHODS
      //#################################################################################//

      /// @brief The method called to get the 'AlarmTime' for alarm 'number'
      /// @remarks This method reads the alarm values from the RTC and updates the local
      ///          field values for the alarm selected.
      /// @param number The alarm number: 1 or 2. Alarm 2 is the default alarm.
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @sideeffect This reads from the RTC and updates the local alarm(1/2) field value.
      /// @design This method was included as a workaround to allow the user to get alarm 1
      ///         without breaking the property pattern for the Alarm, so no '_' after get....
      /// @author Chris-80 (2025/07)
      AlarmTime GetAlarm(int number);

      /// @brief Methods to register/unregister a callback function at every second.
      /// @param callback The function to call every second with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @author Chris-70 (2025/07)
      bool registerTimeCallback(void (*callback)(DateTime));
      bool unregisterTimeCallback(void (*callback)(DateTime));

      /// @brief  Methods to register/unregister a callback function for the alarm.
      ///         The callback function is called when the alarm is triggered.
      /// @param callback The function to call when the alarm is triggered with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @author Chris-70 (2025/07)
      bool registerAlarmCallback(void (*callback)(DateTime));
      bool unregisterAlarmCallback(void (*callback)(DateTime));

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

      /// @brief Method to flash the 'ledNum' ON/OFF for ~(1 sec / frequency). with an ON/OFF 'dutyCycle' 0-100
      /// @param ledNum The output pin number to flash ON/OFF (HIGH/LOW).
      /// @param repeat The number of times to flash ON/OFF, each takes ~(1 sec / frequency).{1}
      /// @param dutyCycle The percentage of time to keep the LED ON (HIGH), 0 - 100.       {50}
      /// @param frequency The number of times per second to flash ON/OFF (Hz) 1 - 25.       {1}
      /// @note The LED must be wired CC, where the LED is on when the pin goes HIGH.
      ///       The LED is in the OFF (LOW) state when this method returns.
      /// @remarks A duty cycle outside of the range 10 - 90 or a frequency > 10 will not 
      ///          appear to be flashing. Use a duty cycle between 25 - 75 and a frequency between 1 - 5
      /// @author Chris-70 (2025/08)
      void FlashLed (uint8_t ledNum, uint8_t repeat = 1, uint8_t dutyCycle = 50, uint8_t frequency = 1);

      /// @brief The method to read the time from the RTC (wrapper for RTC.now()). 
      /// @return A DateTime object containing the current time read from the RTC.
      DateTime ReadTime();

      #if STL_USED
      /// @brief Play a specific melody by its registry id.
      /// @param id The id of the melody in the melodyRegistry to play.
      /// @return True if the id was valid and melody played, false if id was invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      bool PlayMelody(size_t id);
   
      /// @brief Play a specific melody by reference.
      /// @param melody A reference to the vector of `Note` objects to play.
      /// @author Chris-70 (2025/09)
      void PlayMelody(const std::vector<Note>& melody);
   
      /// @brief Register a melody in the melody registry. 
      /// @remarks ID 0 is always the default melody stored in ROM (flash memory).  
      ///          The ID can be used as the alarm melody for a given alarm.
      /// @param melody A reference to the vector of Note objects to register.
      /// @return The ID of the registered melody in the registry.
      /// @see set_Alarm()
      /// @see set_Melody()
      /// @see PlayMelody(size_t id)
      /// @see GetMelodyById()
      /// @author Chris-70 (2025/09)
      size_t RegisterMelody(const std::vector<Note>& melody);
   
      /// @brief Get a melody from the registry by its ID (returned from `RegisterMelody()`).
      /// @param id The id of the melody in the registry.
      /// @return A reference to the melody vector, or the default melody if id is invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      const std::vector<Note>& GetMelodyById(size_t id) const;
      #else
      /// @brief Method to change the alarm melody with the melody Note array: `melodyArray`.
      /// @details Changes the default alarm melody to the given `melodyAlarm`. If the 
      ///          `melodyArray` is nullptr or `melodySize` is 0; the default melody is used.
      /// @param melodyArray The array of melody notes to play, each note is a frequency in Hz.
      /// @param melodySize The size (i.e. number of notes) of the melody array.
      /// @return Flag: true - success, false - failure (e.g. if the array is null or size is 0).
      /// @note   This method is only available for Arduino UNO R3 (or any board that doesn't 
      ///         use the Standard Template Libraries).    
      ///         For all boards that use the Standard Template Libraries (i.e. STL_USED == true):  
      ///         1. Register your melody: size_t RegisterMelody(const vector<Note>& melody);  
      ///         2. Set the current melody: void set_Melody(size_t id);   
      /// @code{.cpp}
      ///         vector<Note> myMelody = {{Note(NOTE_A4,  500), Note(NOTE_A4,  500), Note(NOTE_A4,  500), Note(NOTE_F4,  333)}};
      ///         size_t regNum = RegisterMelody(myMelody);
      ///         set_Melody(regNum);
      /// @endcode
      /// @author Chris-70 (2025/07)
      bool SetAlarmMelody(Note *melodyArray, size_t melodySize);
      #endif

   
   protected:
      /// @brief Enum class to classify the different settings types/levels in the settings menu. Type: uint8_t
      enum class SettingsType : uint8_t 
            { 
            Undefined,     ///< Error: value of 0, not in settings menu.
            TimeOptions,   ///< Time options: 12 or 24 hour mode; Cancel
            Hours,         ///< Setting the hours value.
            Minutes,       ///< Setting the minutes value.
            Seconds,       ///< Setting the seconds value (time only).
            AlarmStatus    ///< Setting the alarm status: ON; OFF; Cancel
            };

      /// @brief Enum class to define the current hour color mode in use. Type: uint8_t
      enum class HourColor : uint8_t 
            { 
            Hour24 = 0,    ///< 24-hour mode, use OnHour colors for hours.
            Am,            ///< AM hour colors when the AM indicator is OFF (i.e. Black)
            Pm             ///< PM hour colors, same as Hour24, default hour colors.
            };

      /// @brief Enum class to define the index to different LED patterns. Type: uint8_t
      /// @remarks The enum values correspond to the first index of the 2D `ledPatternsP` 
      ///          array of `CRGB` colors stored in flash memory.
      /// @note  The `endTAG` is equal to the number of patterns defined (7 or 8) and must
      ///        be the last entry in the enum. To reduce the use of flash memory for overhead,
      ///        all full shield patters are stored together on the 2D array. The enum acts
      ///        as the index to each pattern/color set so care must be taken to ensure
      ///        the correct pattern/color set is stored at the correct index.
      enum class LedPattern : uint8_t
            { 
            onColors = 0,  ///< The LED colors when ON (hours; minutes; seconds).
            offColors,     ///< The LED colors when OFF (usually Black; no power).
            onText,        ///< The big Green `O` for the On pattern.
            offTxt,        ///< The big RED sideways `F` for the Off pattern.
            xAbort,        ///< The big Pink `X' [❌] for the abort/cancel pattern.
            okText,        ///< The big Lime 'checkmark' [✅] for the okay/good pattern.
            rainbow,       ///< The colors of the rainbow on the diagnal pattern.
            #if ESP32_WIFI
            wText,         ///< The big RoyalBlue 'W' [📶] for the WPS / WiFi pattern.
            #endif
            endTAG         ///< The end marker, also equal to the number of patterns defined (7 or 8).
            };

      /// @brief Protected Constructor for the BinaryClock class. This class is 
      ///        implementing the singleton pattern, only one instance is available.
      /// @remark This constructor is protected to prevent direct instantiation.
      ///         Use the `BinaryClock::GetInstance()` method to access the singleton instance.
      /// @see get_Instance()
      BinaryClock();

      /// @brief Destructor for the BinaryClock class. This destructor performs
      ///        cleanup of: the interrupts; RTC Square Wave output; FastED.
      virtual ~BinaryClock();

      /// @addtogroup singleton Singleton Pattern
      /// Singleton pattern - Disable these constructors and assignment operators.
      /// Constructor is protected and access is through `get_Instance()` only.
      /// @see get_Instance()
      /// @{
      BinaryClock (const BinaryClock&) = delete;            ///< Disable copy constructor
      BinaryClock& operator=(const BinaryClock&) = delete;  ///< Disable assignment operator
      BinaryClock (BinaryClock&&) = delete;                 ///< Disable move constructor
      BinaryClock& operator=(BinaryClock&&) = delete;       ///< Disable move assignment operator
      /// @}

      /// @brief This method is to isolate the code needed to initialize the Buttons.
      ///        The 'ButtonState.onValue' determines the type: INPUT_PULLUP/DOWN.
      /// @author Chris-70 (2025/07)
      void InitializeButtons();

      /// @brief This method is to isolate the code needed to setup for the RTC.
      /// @author Chris-80 (2025/07)
      bool SetupRTC();

      /// @brief This method is to isolate the code needed to setup the alarm.
      /// @author Chris-80 (2025/07)
      void SetupAlarm();

      /// @brief This method is to isolate the code needed to setup the FastLED library.
      /// @param testLEDs - Flag: Display the LED test patterns.
      /// @author Chris-80 (2025/07)
      void SetupFastLED(bool testLEDs);

      #if FREE_RTOS
      /// @brief This method runs the task to handle the RTC time and alarm. It waits for the 
      ///        1 Hz RTC Interrupt, calls the 'TimeDispatch()' method to read the RTC time and
      ///        check if the alarm has fired. 
      /// @note  This method isn't used on boards that don't run FreeRTOS, they just call the
      ///        'TimeDispatch()' method from within the 'loop()' method.
      void TimeTask();
      #endif
      
      /// @brief This method handles the reading of the time from the RTC and checks if the 
      ///        alarm has been triggered (when set). 
      /// @returns bool - Flag indicating the interrupt had fired and time was read from the RTC.
      /// @design  This method exists to be called by boards that don't have FreeRTOS.
      ///          Instead of executing the code in 'TimeTask()' the code is encompassed in 
      ///          this method so that it can be called from within the 'loop()' method.
      bool TimeDispatch();

      /// @brief This method is used to get the settings type based on the options and level.
      /// @param options The current settings options, e.g. TimeOptions (1), AlarmStatus (3).
      /// @param level The current settings level, e.g. 1 - 4.
      /// @return The SettingsType enum value that corresponds to the options and level.
      SettingsType GetSettingsType(int options, int level);

      // ################################################################################
      // NEW METHODS - 
      // ################################################################################

      /// @brief This method is called to service the user callback function with the associated time.
      /// @param triggerFlag The flag that indicates if the callback was fired.
      /// @param time The associated DateTime object to pass to the callback function (e.g. alarm time / current time).
      /// @param callback The user callback function to call with the associated DateTime.
      /// @details This method is called when the RTC 1 Hz signal is triggered (time) or the alarm has triggered.
      /// @author Chris-70 (2025/07)
      void CallbackFtn(volatile bool& triggerFlag, DateTime time, void (*callback)(DateTime));

      /// @brief This method is called to dispatch the callback functions for the alarm and time.
      /// @details This method calls the 'CallbackFtn()' when the associated trigger is set and
      ///          the user has registered a callback function for the trigger.
      /// @author Chris-70 (2025/07)
      void CallbackDispatch();

      #if FREE_RTOS
      /// @brief This method is called to run the callback task in a separate thread.
      /// @details This method is called in a separate thread on UNO boards that run FreeRTOS.
      ///          This task just calls 'CallbackDispatch()' and briefly pauses execution in a loop.
      /// @author Chris-70 (2025/07)
      void CallbackTask();
      #endif

      /// @brief This method is called when the BinaryClock has died. It signals **CQD NO RTC** 
      ///        (Come Quick Distress NO RTC) in Morse code on the builtin led forever. 
      /// @details The LEDs on shield are turned OFF first (in case the shield is attached).
      ///        This is called for a catastrophic failure such as missing/failed RTC chip. 
      ///        If the RTC (i.e. shield) is attached/found the board is rebooted.
      ///        This is where the BinaryClock software goes to die.
      /// @note  We do **NOT** use the actual distress code: _SOS_, as this is reserved for actual
      ///        life critical emergencies. We use the old distress code: CQD ["-.-.  --.-  -.."] 
      ///        which means: 'Come Quick Distress`. This was replaced more than a century ago by 
      ///        SOS. **SOS** is the international distress signal for life critical emergencies.
      /// @author Chris-70 (2025/07)
      void PurgatoryTask(const char* message = nullptr);

      /// @brief This method is called to reset the BinaryClock and restart the program.
      /// @details This method is called when the BinaryClock needs to be reset, e.g. after a fatal error.
      ///          It calls the reset function at address 0, which is the start of the program.
      /// @author Chris-70 (2025/08)
      void(*resetBoard) (void) = 0; // Declare reset function at address 0

      #if DEV_CODE
      /// @brief This method is called to display all the registers of the RTC chip.
      ///        The DS3231 registers 0x00 through 0x13 are dumped in: Hex; Binary; and Decimal.
      void DisplayAllRegisters();
      #endif

      #if SERIAL_OUTPUT
      /// @brief This method is called to format the hour for display in 12-hour or 24-hour format.
      /// @param hour24 The hour in 24-hour format (0-23).
      /// @param is12HourFormat Flag indicating if 12-hour format is requested.
      /// @param buffer The character buffer to store the formatted hour.
      /// @param size The size of the buffer.
      /// @return A pointer to the given `buffer` containing the formatted hour string.
      char* FormatHour(int hour24, bool is12HourFormat, char* buffer, size_t size);
      #endif

      #if HARDWARE_DEBUG
      /// @brief This method is called to check the hardware debug buttons/switches and set the serial output level.
      /// @author Chris-80 (2025/07)
      void CheckHardwareDebugPin();
      #endif

      // ################################################################################
      // ORIGINAL METHODS - 
      // ################################################################################

   private:
   TEST_PROTECTED
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

      /// @brief The method called to convert the time to binary and update the LEDs.
      /// @details This method converts the current time to binary and updates the LEDs 
      ///          using the color values defined in the arrays 'OnColor' and 'OffColor'
      /// @param hoursRow The value for the top, to display the hour LEDs (16-12).
      /// @param minutesRow The value for the middle, to display the minute LEDs (11-6).
      /// @param secondRow The value for the bottom, to display the second LEDs (5-0).
      /// @param use12HourMode Flag indicating whether to use 12-hour format.
      /// @see set_Brightness() for the brightness of the LEDs.
      /// @see displayLedBuffer() for displaying the full LED buffer as defined.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void convertDecToBinaryAndDisplay(int hoursRow, int minutesRow, int secondsRow, bool use12HourMode = false);

      /// @brief The method called to display the LED buffer on the LEDs.
      /// @param ledBuffer The array buffer containing the LED colors to display.
      /// @details This method just copies the given 'ledBuffer' contents directly to the 
      ///          FastLED buffer and displays it.
      /// @author Chris-70 (2025/07)
      void displayLedBuffer(const fl::array<CRGB, NUM_LEDS>& ledBuffer);

      /// @brief The method called to display the LED buffer on the LEDs for
      ///        the given `patternType`.
      /// @param patternType The LED pattern type to display.
      /// @author Chris-70 (2025/08)
      void displayLedBuffer(LedPattern patternType);

      /// @brief Helper method to return the pointer to the `patternType` in the `ledPatternsP` array.
      /// @param patternType The LED pattern type to display.
      /// @author Chris-70 (2025/08)
      const CRGB* patternLookup(LedPattern patternType);

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

      /// @brief Helper function to fill a string with a repeated character.
      /// @remarks This trades a bit of speed for flash memory savings by 
      ///          creating the string on the fly. If the string is local
      ///          then it's just temporary ram usage.
      /// @param ch The character to repeat.
      /// @param repeat The number of times to repeat the character.
      /// @return A String filled with the repeated character.
      static String fillStr(char ch, byte repeat);

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

   private:
      /// @brief This method is called to display the LED pattern, from `ledPatternsP`, for the specified index.
      /// @remarks This method retrieves the LED pattern from flash memory and displays it on the shield.
      ///          The `ledPatternsP` is a 2D array in flash memory that has all the predefined LED patterns and 
      ///          colors for the binary clock.
      /// @param patternIndex The index of the `ledPatternsP` flash memory for the LED pattern.
      void displayLedPattern(LedPattern patternType);

      // ################################################################################
      // FIELDS - 
      // ################################################################################

   public:         

      /// @brief The pin number to use for the heartbeat (if enabled) or to signal errors.
      /// @note  The LED must be wired CC, the pin will go HIGH to turn it ON.
      static uint8_t HeartbeatLED;

   protected:
      RTCLibPlusDS3231 RTC;                        // Create RTC object using forked Adafruit RTCLib library

      /// @brief Default: Colors for the LEDs when ON, Seconds, Minutes and Hours
      /// @details The default colors are Hours: Blue; Minutes: Green; and Seconds: Red 
      ///@note The hours are defined by `OnHour` color array. These are the colors used for 24 hour mode and for PM.
      ///      AM is defined by the `OnHourAM` color array when `AmColor` is Black.  This is to remove ambiguity for
      ///      hour 12. Is it noon in 24 hour mode or midnight in 12 hour mode? Without an AM indicator, who knows?
      static fl::array<CRGB, NUM_LEDS> OnColor;

      /// @brief Default: Colors for the LEDs Seconds, Minutes and Hours, when OFF (Usually Black i.e. No Power.)
      /// @note  Using any color other than Black means the LEDs will be consuming power at all times.
      static fl::array<CRGB, NUM_LEDS> OffColor;

      /// @brief Default: Colors for the AM hour LEDs when `AmColor` is Black. This is to show the clock is in 12 hour
      ///        mode when there is no AM indicator. 
      /// @remarks When the AM indicator color is Black, there is no way to differentiate between 12 noon in 
      ///          24 hour mode and 12 midnight in 12 hour mode. To remove this ambiguity, the AM hours are shown
      ///          in a different color, e.g. DeepSkyBlue.
      /// @see OnHour
      static fl::array<CRGB, NUM_HOUR_LEDS> OnHourAM; 

      /// @brief Default: LED colors for the PM hours or for all 24 hours.
      /// @remarks The default hour colors are are used for 24 hour mode and just for PM in 12 hour mode when 
      ///          `AmColor` is Black. When the `AmColor` is NOT Black, there is no ambiguity between
      ///          12 midnight in 12 hour mode and 12 noon in 24 hour mode.
      static fl::array<CRGB, NUM_HOUR_LEDS> OnHour;

      static CRGB PmColor;                         ///< Color for the PM indicator LED, e.g. Indigo.
      static CRGB AmColor;                         ///< Color for the AM indicator (Usually Black/OFF).

      // The UNO Compiler doesn't support this C++ style object initialization, move it to the constructor.
      AlarmTime Alarm1;                            ///< DS3232 alarm 2, includes seconds in alarm.
      AlarmTime Alarm2;                            ///< Default alarm, seconds set at 00.

      volatile bool RTCinterruptWasCalled;         ///< Flag: The RTC interrupt was triggered.
      volatile bool CallbackAlarmTriggered;        ///< Flag: The 'Alarm' callback needs to be called.
      volatile bool CallbackTimeTriggered;         ///< Flag: The 'Time'  callback needs to be called.

      const char* timeFormat24 = "hh:mm:ss";       ///< 24-hour time format string: 00:00:00 to 23:59:59
      const char* timeFormat12 = "HH:mm:ss AP";    ///< 12-hour time format string: 12:00:00 AM to 11:59:59 PM
      const char* timeFormat = timeFormat24;       ///< Pointer to the current format string for the time.
      const char* alarmFormat24 = "hh:mm";         ///< 24-hour alarm format string: 00:00 to 23:59
      const char* alarmFormat12 = "HH:mm AP";      ///< 12-hour alarm format string: 12:00 AM to 11:59 PM
      const char* alarmFormat = alarmFormat24;     ///< Pointer to the current format string for the alarm.

      // Note durations: 4 = quarter note, 8 = eighth note, etc.:
      // Some notes durations have been changed (1, 3, 6) to make them sound better
      static const unsigned long NoteDurations[] PROGMEM;   ///< Note durations array, unsigned long array (64 bits)
      static const unsigned      MelodyAlarm[] PROGMEM;     ///< Melody for alarm, unsigned integer array (32 bits)
      static const size_t        MelodySize;                ///< Size of the melody array
      static const size_t        NoteDurationsSize;         ///< Size of the note durations array

   private:
   TEST_PROTECTED

      CRGB leds[NUM_LEDS] = {0};          ///< Array of LED colors to display the current time
      bool binaryArray[NUM_LEDS];         ///< Serial Debug: Array for binary representation of the time

      fl::array<CRGB, NUM_LEDS>& onColors;         ///< Reference to the current ON  colors.
      fl::array<CRGB, NUM_LEDS>& offColors;        ///< Reference to the current OFF colors.
      fl::array<CRGB, NUM_HOUR_LEDS>& onHour;      ///< Reference to the color for the hours (except AM).
      fl::array<CRGB, NUM_HOUR_LEDS>& onHourAM;    ///< Reference to the color array for the AM hours.

      // Define a MACRO to declare the buttons and initialize the `ButtonState` values.
      // A macro to reduce cut-n-paste errors so initialization is always correct based on
      //         if the button input pin is pulled LOW (CC) or HIGH (CA) in the OFF state.
      // The `onValue` is set to the value that the button pin sees when pressed, 
      //      HIGH (i.e. CC_ON, button connects to VCC when pressed) or 
      //      LOW  (i.e. CA_ON, button connects to ground when pressed).
      // `NAME` - The suffix to add to `button` (i.e. `buttonNAME`) to create the ButtonState object.
      // `PIN`  - The pin number that the button is connected to.
      // `TYPE_CC_CA` - `CC` when the OFF state is LOW  (i.e. pulled HIGH when pressed), 
      //              - `CA` when the OFF state is HIGH (i.e. pulled LOW  when pressed)..
      #define DECLARE_BUTTON(NAME, PIN, TYPE_CC_CA) \
            ButtonState button##NAME = { .pin = PIN, .state = TYPE_CC_CA##_OFF, .lastRead = TYPE_CC_CA##_OFF, \
                                         .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = TYPE_CC_CA##_ON };

      // The 3 buttons used to control the Binary Clock Shield menu for setting the time and alarm.
      DECLARE_BUTTON(S1, S1, CC)         ///< Declare 'ButtonState::buttonS1'
      DECLARE_BUTTON(S2, S2, CC)         ///< Declare 'ButtonState::buttonS2'
      DECLARE_BUTTON(S3, S3, CC)         ///< Declare 'ButtonState::buttonS3'

      #if DEV_BOARD
      DECLARE_BUTTON(DOut, 17, CC) // *** DEBUG ***
      #endif
            
      #if HW_DEBUG_SETUP
      DECLARE_BUTTON(DebugSetup, DEBUG_SETUP_PIN, CC)   ///< Declare 'ButtonState::buttonDebugSetup'
      #endif

      #if HW_DEBUG_TIME
      DECLARE_BUTTON(DebugTime, DEBUG_TIME_PIN, CA)     ///< Declare 'ButtonState::buttonDebugTime'
      #endif

      #undef DECLARE_BUTTON   // Undefine, we only needed it here to write the declarations without errors.

      DateTime time;                         ///< Current time from the RTC, updated every second.
      DateTime tempTime;                     ///< Temporary time variable used when setting the time.
      AlarmTime tempAlarm;                   ///< Temporary Alarm used when setting the alarm.
      bool tempAmPm = false;                 ///< Temporary flag for 12/24 Hr. mode when setting time.
      bool amPmMode = false;                 ///< Flag: Indicates if the clock is in 12-hour AM/PM, or 24 Hr mode.
      int countButtonPressed = 0;            ///< Counter for button pressed during time/alarm settings
      bool callbackAlarmEnabled = false;     ///< Flag: The 'Alarm' callback is enabled (i.e. is not nullptr) or not.
      bool callbackTimeEnabled  = false;     ///< Flag: The 'Time'  callback is enabled (i.e. is not nullptr) or not.
      bool rtcValid             = false;     ///< Flag: The RTC was found and initialized.
      void (*alarmCallback)(DateTime) = nullptr; ///< Callback function for alarm triggers.
      void (*timeCallback)(DateTime)  = nullptr; ///< Callback function for time trigger (1 Hz frequency).

      unsigned long debounceDelay = DEFAULT_DEBOUNCE_DELAY; ///< The debounce time for a button press.
      bool pixelsPresent = false;            ///< Flag: Indicates if the shield is attached (or just a dev. board).

      // Variables that store the current settings option
      int settingsOption = 0;               ///< Time = 1, Alarm = 3  
      int settingsLevel = 0;                ///< Hours = 1, Minutes = 2, Seconds / On/Off Alarm = 3
      static const uint8_t repeatModeTable[(uint8_t)(AlarmTime::Repeat::endTag)][2];

      #define REPEAT_MODE_TABLE_SIZE (sizeof(repeatModeTable) / sizeof(repeatModeTable[0]))
   
      int alarmRepeatMax = DEFAULT_ALARM_REPEAT;   ///< Maximum alarm sound repeat count
      int alarmRepeatCount = 0;                    ///< Current alarm sound repeat count
      byte brightness = DEFAULT_BRIGHTNESS;        ///< Brightness of the LEDs, 0-255 (20 - 60 is a good range).

      char buffer[64] = { 0 };                     ///< Buffer for the DateTime string conversions

      bool isSerialSetup = (SERIAL_SETUP_CODE) && (DEFAULT_SERIAL_SETUP); ///< Serial setup flag
      bool isSerialTime  = (SERIAL_TIME_CODE)  && (DEFAULT_SERIAL_TIME);  ///< Serial time  flag 

      bool isAmBlack = true;     ///< Flag: Controls if we switch the hour colors for AM/PM.
      bool switchColors = false; ///< Flag to perform the switch of OnHour and OnHourAM hour colors.
      HourColor curHourColor = HourColor::Hour24; ///< Current ON hosur colors in use.

      /// @var CRGB ledPatternsP[][]
      /// @brief A 2D array of LED colors and patterns. stored in flash memory, for the shield.
      ///          These are the default ON/OFF colors for displaying the time as well as
      ///          all the patterns used in the settings menu for time and alarm.
      /// @details The enum `LedPattern` is the index to the color/pattern for that display.
      /// @par     `LedPattern::onColors':  
      ///          `OnColors` The default colors are Hours: Blue; Minutes: Green; and Seconds: Red 
      ///          The hours are defined by `OnHour` color array. These are the colors used for 
      ///          24 hour mode and for PM. AM is defined by the `OnHourAM` color array when the 
      ///          `AmColor` is Black.  This is to remove ambiguity for hour 12. Is it noon in 
      ///          24 hour mode or midnight in 12 hour mode? Without an AM indicator, who knows?
      /// @par     `LedPattern::offColors`:    
      ///          `OffColors` for the LEDs when OFF (Usually Black i.e. No Power.)
      ///          Using any color other than Black means the LEDs will be consuming power at all times.
      /// @par     `LedPattern::onText`: 
      ///          `OnText` The screen shaped in an 'O' for 'On' when setting the alarm to 
      ///          ON in the alarm menu.
      /// @par     `LedPattern::offTxt`: 
      ///          `OffTxt` The screen shaped in a RED sideways 'F' for 'oFF' when setting the 
      ///          alarm to OFF in the alarm menu.
      /// @par     `LedPattern::xAbort`:   
      ///          `XAbort` The screen shaped in a big Pink (Fuchsia) 'X' [❌] for abort/cancel.
      ///          This is used to cancel the Time and Alarm settings and exit without making any changes.
      ///          This is also displayed, after the Rainbow (saving/exit) screen to signal nothing saved.
      /// @par     `LedPattern::okText`: 
      ///          `OkText` The screen shaped in a big Lime 'checkmark' [✅] for okay/good.
      ///          This is used to signal that the settings have been saved successfully.
      /// @par     `LedPattern::rainbow`:  
      ///          `Rainbow` The screen shaped in a big Rainbow of colors across all LEDs.
      ///          This is displayed after the Time or Alarm settings has ended and the
      ///          program is saving/restoring the settings. This is followed by either
      ///          the 'checkmark' [✅] for settings saved or the 'X' [❌] for no changes.
      /// @par     `LedPattern::wText`:  
      ///          `WText` The screen shaped in a big Blue 'W' [📶] for WPS / WiFi
      ///          This is used to signal that the WiFi needs to setup (e.g. WPS) and is only available
      ///          when the device supports WiFi (i.e. ESP32_WIFI is defined).
      /// @see `LedPattern`
      static const CRGB ledPatternsP[static_cast<uint8_t>(LedPattern::endTAG)][NUM_LEDS] PROGMEM;

      /// @var CRGB hourColorsP[][]
      /// @brief A 2D array for colors for just the hours. The `OnHourAM` is the alternative colors used 
      ///        for the AM hour LEDs when `AmColor` is Black. This is to show the clock is in 12 hour
      ///        mode when there is no AM indicator. The `OnHour` is the standard hour colors generally used.
      /// @details The `OnColor` hour section is replaced with either the `OnHourAM` or `OnHour` colors
      ///          when the time switches from AM to PM and the flag `isAmBlack` is `true`.
      /// @remarks When the AM indicator color is Black, there is no way to differentiate between 12 noon in 
      ///          24 hour mode and 12 midnight in 12 hour mode. To remove this ambiguity, the AM hours are shown
      ///          in a different color, e.g. DeepSkyBlue.
      /// @par     OnHourAM:
      ///          Index 0 - `OnHourAM`
      /// @par     OnHour:
      ///          Index 1 - `OnHour`
      static const CRGB hourColorsP[][NUM_HOUR_LEDS] PROGMEM;

      static const uint8_t ledPatternCount;  ///< Number of patterns in the 2D array (i.e. value of LedPattern::endTAG).

      static const CRGB* onColorP;     ///< Pointer to the `OnColor`  colors (index 0) in `ledPatternsP`
      static const CRGB* offColorP;    ///< Pointer to the `OffColor` colors (index 1) in `ledPatternsP`
      static const CRGB* onHourAmP;    ///< Pointer to the `OnHourAM` colors (index 0) in `hourColorsP`
      static const CRGB* onHourP;      ///< Pointer to the `OnHour`   colors (index 1) in `hourColorsP`

      #if SERIAL_SETUP_CODE
      // Setup strings for seria output that are used multiple times. Balance between flash and ram usage.
      static const String STR_SEPARATOR;              ///< Repeated separator string, generated at runtime.
      static const String STR_BARRIER;                ///< Repeated barrier string, generated at runtime.
      static const char PROGMEM STR_TIME_SETTINGS[];  ///< Repeated time settings string, stored in flash memory.
      static const char PROGMEM STR_ALARM_SETTINGS[]; ///< Repeated alarm settings string, stored in flash memory.
      static const char PROGMEM STR_CURRENT_TIME[];   ///< Repeated current time string, stored in flash memory.
      #endif
      /// @brief Time to wait after serial time button goes off before stopping the serial output.
      ///        Set to a long delay if using a momentary button, keep short for a switch. This
      ///        allows a button to be pressed, released and you still get output for 'debugDelay' ms.
      unsigned long debugDelay = DEFAULT_DEBUG_OFF_DELAY; 

      #if STL_USED
      std::vector<Note> defaultMelody;                    ///< Default melody created from PROGMEM arrays
      std::vector<std::reference_wrapper<const std::vector<Note>>> melodyRegistry; ///< Registry of melody references
      size_t currentMelody;                               ///< Index to the current melody in melodyRegistry
      #else
      // Replace the old melody fields:
      // unsigned* melodyAlarm;        ///< Pointer to the melody array
      // size_t melodySize;            ///< Size of the melody array
      // unsigned long* noteDurations; ///< Pointer to the note durations array
      // size_t noteDurationsSize;     ///< Size of the note durations array
      
      bool isDefaultMelody = true;     ///< Flag: using the default alarm melody or not.
      const Note* alarmNotes;          ///< Pointer to the combined alarm notes array
      size_t alarmNotesSize;           ///< Size of the alarm notes array
      #endif
   
      static const Note AlarmNotes[] PROGMEM;     ///< The default alarm melody, an array of `Notes`
      static const size_t AlarmNotesSize;         ///< Size of the AlarmNotes array
      }; // END Class BinaryClock
   }  // END namespace BinaryClockShield
#endif
