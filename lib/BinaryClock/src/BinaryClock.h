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
///  (*) Requires hardware board modification to use LED output pin. Wire pin 15 on the D1-R32 board to
///      the SHIELD pin that connects to location `A3`/`34`. This is where the LED data pin is connected to.
///      See the 'README.md' file: https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md 
///      [Hardware Modifications section](https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md#hardware-modifications).
/// (**) The shield is connected to and uses the alternate SDA (A4) and SCL (A5) pins: PC4; and PC5.
///      These pins are located just below the RESET button and above the `AREF` pin.
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
/// This project has been inspired from the original Example; "11-BinaryClock-24H-RTCInterruptAlarmButtons.ino" file as published
/// on the Binary Clock Shield for Arduino GitHub repository: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
/// The original file was fully refactored to be encapsulated in a group of classes: IBinaryClock - The interface class;
/// BinaryClock - The implementation and main class; BCButton - Class to manage the buttons; BCMenu - Class to manage the
/// alarm and time settings, this class retains much of the original code. These classes encapsulate all the functionality
/// available on "Binary Clock Shield for Arduino UNO" designed and built by Marcin Saj." Modifications were made to support
/// multiple UNO boards including ESP32 based UNO platforms that would allow for new functionality, such as WiFi time.
/// For all boards, there is greater flexibility by the user at runtime for color selection and melodies used by the alarm as well as
///  an improved user experience when setting the alarm and time including support for 12 hour mode.
///       
/// The original goal of using an ESP32 based UNO board was to allow the RTC to be connected to a NTP server over WiFi. The code
/// for the WiFi connection is encapsulated in its own class, 'BinaryClock_NTP', and requires a board with an ESP32 WiFi chip.
/// WPS is used to connect to a WiFi network and store the credentials in the ESP32 flash memory so future connections are
/// made automatically without user intervention. The WiFi connection also allows the user to change the LED colors
/// and melodies used for the alarm at runtime, without needing to recompile the code.  
///    

#pragma once
#ifndef __BINARYCLOCK_H__
#define __BINARYCLOCK_H__

#define DEBUG_OUTPUT true

#include <Arduino.h>             /// Arduino core library. This needs to be the first include file.

#include "BinaryClock.Defines.h" /// BinaryClock project-wide definitions and MACROs.
#include "BinaryClock.Structs.h" /// Global structures and enums used by the Binary Clock project.

#include "IBinaryClock.h"        /// The pure interface class that defines the minimum supported features.
#include "BCMenu.h"              /// Binary Clock Settings class: handles all settings and serial output.
#include "BCButton.h"            /// Binary Clock Button class: handles all button related functionality.

#include <FastLED.h>             /// For control of the WS2812B LEDs. (https://github.com/FastLED/FastLED)
#include <fl/array.h>            /// For fl::array used for the LEDS.
#include "fl/namespace.h"        /// For fl:: namespace used in the FastLED library.

#include <RTClib.h>              /// For the `RTC_DS3231` & `DateTime` classes (https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus)
#include <Streaming.h>           /// Streaming serial output with `operator<<` (https://github.com/janelia-arduino/Streaming)

#include "pitches.h"             /// Needed to create the pitches. Library: https://arduino.cc/en/Tutorial/ToneMelody

#if STL_USED
   // STL classes required to be included (when using the STL):
   #include <vector>
   #include <functional>
#endif

#ifdef TESTING                   ///< Changes needed for unit testing of this code.
   #define TEST_VIRTUAL virtual        ///< Virtul methods for unit testing ony.
   #define TEST_PROTECTED protected:   ///< Access specifier for unit testing ony.
#else
   #define TEST_VIRTUAL                ///< Virtual methods for unit testing, removed otherwise.
   #define TEST_PROTECTED              ///< Access specifier for unit testing, removed otherwise.
#endif

/// @namespace BinaryClockShield
/// @brief
/// Namespace for all Binary Clock software used in this project.
/// Protect from any possible name clashes by putting all code in its own namespace.
namespace BinaryClockShield
   {
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
   ///          targeted an Arduino UNO board, this is designed to include ESP32 based UNO boards, such as 
   ///          Adafruit Metro ESP32 S3 (https://www.adafruit.com/product/5500), Arduino UNO R4 Minima (no WiFi)
   ///          (https://store.arduino.cc/products/uno-r4-minima), Arduino UNO R4 WiFi (which has an ESP32-S3)
   ///          (https://store.arduino.cc/products/uno-r4-wifi), the Wemos D1 R32 UNO or the new ESP32-S3 UNO. 
   ///          The idea is to connect to a NTP server over WiFi for time setting.
   /// @remarks This class is designed to be used with the excellent 'Binary Clock Shield for Arduino' by 
   ///          Marcin Saj (available from: https://nixietester.com/product/binary-clock-shield-for-arduino/), 
   ///          original source code: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino  
   ///          This class uses a fork of the Adafruit RTCLib library, RTClibPlus, for the RTC functionality 
   ///          (https://github.com/adafruit/RTClib) in place of the original DS3232RTC library by 
   ///          Jack Christensen (https://github.com/JChristensen/DS3232RTC) used in the original code.  
   ///          One big reason for this is the inclusion of the classes `DateTime` and `TimeSpan`
   ///          which closely resemble the C# classes. The implementation of `DateTime` keeps the
   ///          time in individual bytes for Year; Month; Day; Hours; Minutes; and Seconds which is
   ///          close to the format that the RTC uses. This class has been fully documented using `Doxygen`.   
   /// @design  This class was designed to be used with multiple Arduino UNO style boards and to encapsulate
   ///          the base functionality of displaying the time in Binary format on the shield. The class 
   ///          follows the `Singleton Pattern` to ensure that only one instance of the class exists.   
   ///          The class implements the `IBinaryClock` interface to ensure that all required methods are provided.  
   ///          The `BCMenu` class handles all the settings and serial output functionality.  
   ///          The `BCButton` class handles all button related functionality.  
   ///          The `BinaryClock.Defines.h` file contains all the project-wide definitions and MACROs required 
   ///          to support multiple different UNO board types including a user defined `CUSTOM_UNO` board.   
   ///          The `BinaryClock.Structs.h` file contains all the project-wide structures used in this project.   
   ///          The `board_select.h` file is an optional file for all user defines, overrides and  the
   ///          definition of a `CUSTOM_UNO` board. If this file exists, it is included __before__ any defines 
   ///          in `BinaryClock.Defines.h` allowing the user to override defines and MACROs as required.  
   ///          The `pitches.h` file contains the default alarm melody.  
   /// @par     Custom Libraries:  
   ///          - `RTClibPlus`: A fork of the Adafruit RTCLib library with additional features including
   ///                          raw read/write access to the RTC registers and the `DateTime` and `TimeSpan` 
   ///                          classes. The library was modified to support 12 (i.e. AM/PM) time/alarm format 
   ///                          available on the RTC chip.  To accommodate all cultures, the starting 
   ///                          day of the week can be defined at compilation time. The default, first day, 
   ///                          1 is defined as `Monday` in keeping with original code. The day of the week for 
   ///                          `DateTime` and the RTC chip are kept in sync from the selection at compile time.

   ///          - `MorseCodeLED`: A simple library to flash Morse code on an LED. This is used to flash a
   ///                            Morse code error message such as when the shield isn't found/working. 
   ///                            Aside from distinguishing the flashes from a blinky sketch. This allows 
   ///                            for communication of messages that can be understood by 0.001% of people 
   ///                            and by 100% of online Morse code interpreters, and it was a fun exercise.
   /// 
   ///          - `BinaryClockWAN`:  A library to handle all WiFi related functionality including NTP server
   ///                               connections for time synchronization. This library connects to an open 
   ///                               local WiFi, a secure WiFi using WPS, or a secure WiFi using credentials 
   ///                               from the user and stored in the ESP32 flash memory. The library also 
   ///                               handles all NTP related functionality including time synchronization and
   ///                               daylight saving time adjustments based on user settings. 
   ///                               The `BinaryClockWAN` is the main class and uses the other classes for support:  
   ///                               The `BinaryClockWPS` class establishes a connection with an AP using WPS.   
   ///                               The `BinaryClockNTP` class handles all NTP related functionality.   
   ///                               The `BinaryClockSettings` class handles the storing and retrieving of all 
   ///                               settings in NVS.  
   /// @author  Chris-80 (2025/07)
   class BinaryClock : public IBinaryClock
      {
   //#################################################################################//  
   //                            IBinaryClock INTERFACE                               //
   //#################################################################################//   
   public:

      /// @ingroup properties
      /// @{
      /// @brief The property methods called to set/get the current 'Time' property.
      /// @param value The DateTime object containing the current time to set.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @see get_Time()
      /// @see ReadTime()
      /// @author Chris-80 (2025/07)
      void set_Time(DateTime value) override;
      /// @copydoc set_Time()
      /// @return A DateTime object containing the current time.
      /// @see set_Time()
      DateTime get_Time() const override
         { return time; }
      /// @}

      /// @ingroup properties
      /// @{
      /// @brief The property method called to set/get the current 'Alarm' property.
      /// @param value The AlarmTime structure containing the alarm time and status.
      /// @note The AlarmTime structure contains the hour, minute, and status of the alarm
      ///       The status is 0 for inactive, 1 for active.
      ///       Hours are 0 to 23.
      /// @see get_Alarm()
      /// @see GetRtcAlarm(int)
      /// @author Chris-80 (2025/07)
      virtual void set_Alarm(AlarmTime value) override;
      /// @copydoc set_Alarm()
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @see set_Alarm()
      virtual AlarmTime get_Alarm() const override 
         { return Alarm2; }
      /// @}

      /// @ingroup properties
      /// @{
      /// @brief Public Read, Protected Write property pattern for the 'TimeFormat' string property.
      ///        This property returns the current format of the time string used for `format` in 
      ///        the `DateTime::toString(char *buffer, size_t size, const char* format)` method.
      /// @returns A pointer to a constant character string containing the current time format.
      /// @see get_AlarmFormat()
      /// @author Chris-70 (2025-08)
      virtual char* const get_TimeFormat() const override 
         { return (char* const)(TimeFormat == nullptr? DEFAULT_TIME_FORMAT : TimeFormat); }
      /// @}

      /// @ingroup properties
      /// @{
      /// @brief Public Read, Protected Write property pattern for the 'AlarmFormat' string property.
      ///        This property returns the current format of the alarm string used for `format` in
      ///        the `DateTime::toString(char *buffer, size_t size, const char* format)` method.
      /// @returns A pointer to a constant character string containing the current alarm format.
      /// @see get_TimeFormat()
      /// @author Chris-70 (2025-08)
      virtual char* const get_AlarmFormat() const override 
         { return (char* const)(AlarmFormat == nullptr? DEFAULT_ALARM_FORMAT : AlarmFormat); }
      /// @}

      /// @ingroup properties
      /// @{
      /// @brief Property pattern for the 'Is12HourFormat' flag property.
      ///        This property controls whether the time is displayed in 
      ///        12-hour or 24-hour format.
      /// @param value The flag to set (true for 12-hour format, false for 24-hour format).
      /// @see get_Is12HourFormat()
      /// @author Chris-70 (2025-07)
      virtual void set_Is12HourFormat(bool value) override;
      /// @copydoc set_Is12HourFormat()
      /// @return The current flag value (true for 12-hour format, false for 24-hour format).
      /// @see set_Is12HourFormat()
      virtual bool get_Is12HourFormat() const override;

      /// @copydoc set_IsSerialTime()
      /// @return The current flag value (true to display the serial setup menu, false to disable it).
      /// @see set_IsSerialTime()
      virtual bool get_IsSerialTime() const override;

      /// @copydoc set_IsSerialSetup()
      /// @return The current flag value (true to display the serial setup menu, false to disable it).
      /// @see set_IsSerialSetup()
      virtual bool get_IsSerialSetup() const override;

      /// @brief Read only property pattern to get a const reference to the `S1`
      ///        `BCButton` object used for setting time and decrementing a value.
      /// @return A const reference to the `BCButton` for S1 on the shield.
      /// @see get_S2SaveStop()
      /// @see get_S3AlarmInc()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_S1TimeDec() const override { return buttonS1; }

      /// @brief Read only property pattern to get a const reference to the `S2`
      ///        `BCButton` object used for saving a selection or stopping an alarm.
      /// @return A const reference to the `BCButton` for S2 on the shield.
      /// @see get_S1TimeDec()
      /// @see get_S3AlarmInc()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_S2SaveStop() const override { return buttonS2; }

      /// @brief Read only property pattern to get a const reference to the `S3`
      ///        `BCButton` object used for setting alarm and incrementing a value.
      /// @return A const reference to the `BCButton` for S3 on the shield.
      /// @see get_S1TimeDec()
      /// @see get_S2SaveStop()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_S3AlarmInc() const override { return buttonS3; }

      /// @brief Read only property pattern to get the unique Id Name of this Binary Clock instance.
      /// @return A pointer to a constant character string containing the unique identifier name.
      /// @author Chris-70 (2025/10)
      virtual const char* get_IdName() const override { return IBinaryClock_IdName; }
      /// @}

      /// @brief Methods to register/unregister a callback function at every second.
      /// @param callback The function to call every second with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see UnregisterTimeCallback()
      /// @see RegisterAlarmCallback()
      /// @author Chris-70 (2025/07)
      virtual bool RegisterTimeCallback(void (*callback)(const DateTime&)) override
         { return registerCallback(callback, timeCallback, callbackTimeEnabled); }
      /// @copydoc RegisterTimeCallback()
      /// @see RegisterTimeCallback()
      /// @see UnregisterAlarmCallback()
      virtual bool UnregisterTimeCallback(void (*callback)(const DateTime&)) override
         { return unregisterCallback(callback, timeCallback, callbackTimeEnabled); }

      /// @brief  Methods to register/unregister a callback function for the alarm.
      ///         The callback function is called when the alarm is triggered.
      /// @param callback The function to call when the alarm is triggered with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see RegisterTimeCallback()
      /// @see UnregisterAlarmCallback()
      /// @author Chris-70 (2025/07)
      virtual bool RegisterAlarmCallback(void (*callback)(const DateTime&)) override
         { return registerCallback(callback, alarmCallback, callbackAlarmEnabled); }
      /// @copydoc RegisterAlarmCallback()
      /// @see RegisterAlarmCallback()
      /// @see UnregisterTimeCallback()
      virtual bool UnregisterAlarmCallback(void (*callback)(const DateTime&)) override
         { return unregisterCallback(callback, alarmCallback, callbackAlarmEnabled); }

      /// @brief The method called to convert the time to binary and update the LEDs.
      /// @details This method converts the current time to binary and updates the LEDs 
      ///          using the color values defined in the arrays 'OnColor' and 'OffColor'
      /// @param hoursRow The value for the top, to display the hour LEDs (16-12).
      /// @param minutesRow The value for the middle, to display the minute LEDs (11-6).
      /// @param secondRow The value for the bottom, to display the second LEDs (5-0).
      /// @param use12HourMode Flag indicating whether to use 12-hour format or not.
      /// @see set_Brightness() for the brightness of the LEDs.
      /// @see DisplayLedPattern() for displaying the full LED buffer as defined.
      /// @author Chris-80 (2025/07)
      virtual void DisplayBinaryTime(int hoursRow, int minutesRow, int secondsRow, bool use12HourMode = false) override;

      /// @brief The method called to display the LED buffer on the LEDs for
      ///        the given `patternType`.
      /// @details The patterns are defined in the `LedPattern` enum.  
      ///          The `LedPattern` values correspond to the index of the LED buffer,
      ///          `LedPatterns_P` stored in ROM. The method handles the reading from 
      ///          ROM and displaying the pattern on the LEDs.
      /// @param patternType The LED pattern type to display.
      /// @see LedPattern
      /// @see DisplayBinaryTime()
      /// @author Chris-70 (2025/08)
      virtual void DisplayLedPattern(LedPattern patternType) override;

      /// @brief The method called to play the melody from `alarm.melody`.
      /// @details This method plays the melody id defined in the `AlarmTime` structure
      ///          passed as a parameter. The melody id corresponds the the id value
      ///          registered using the `RegisterMelody()` method.
      /// @note    On boards that do not support the STL, such as the Arduino UNO R3,
      ///          the default melody stored in ROM (flash memory) is played. The default
      ///          melody can be changed at runtime for these boards.
      /// @param   alarm - The `AlarmTime` instance to play the indicated melody id.
      /// @see PlayMelody(size_t id)
      /// @author Chris-70 (2025/09)
      virtual void PlayAlarm(const AlarmTime& alarm) const override;

      #if STL_USED
      /// @brief Play a specific melody by its registry id.
      /// @param id The id of the melody in the melodyRegistry to play.
      /// @return True if the id was valid and melody played, false if id was invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      virtual bool PlayMelody(size_t id) const override;

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
      virtual size_t RegisterMelody(const std::vector<Note>& melody) override;

      /// @brief Get a melody from the registry by its ID (returned from `RegisterMelody()`).
      /// @param id The id of the melody in the registry.
      /// @return A reference to the melody vector, or the default melody if id is invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      virtual const std::vector<Note>& GetMelodyById(size_t id) const override;
      #endif

   //#################################################################################//  
   //                         BinaryClock DEFINITION                                  //
   //#################################################################################//   

   //#################################################################################//
   // Public METHODS
   //#################################################################################//
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

      /// @brief The method called to play the alarm melody.
      /// @remarks This method behaves differently for boards that don't use the
      ///          STL library such as the Arduino UNO R3. These boards only
      ///          play the default melody or the melody from `SetAlarmMelody()`.  
      ///          Boards that fully support the STL library play the melody
      ///          registered in the melody registry using the melody id in the
      ///          current alarm.
      /// @see SetAlarmMelody()
      /// @author Chris-70 (2025/09)
      virtual void PlayAlarm() const { PlayAlarm(get_Alarm()); }

      /// @brief The method to read the time from the RTC (wrapper for RTC.now()). 
      /// @return A DateTime object containing the current time read from the RTC.
      DateTime ReadTime() override;

      /// @brief The method called to get the 'AlarmTime' for alarm 'number' from the RTC
      /// @remarks This method reads the alarm values from the RTC and updates the local
      ///          field values for the alarm selected.
      /// @param number The alarm number: 1 or 2. Alarm 2 is the default alarm.
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @sideeffect This reads from the RTC and updates the local alarm(1/2) field value.
      /// @design This method was included as a workaround to allow the user to get alarm 1
      ///         without breaking the property pattern for the Alarm, so no '_' after get....
      /// @author Chris-80 (2025/07)
      AlarmTime GetRtcAlarm(int number);

      /// @brief Method to convert a DateTime value to a string inline. This method takes the format as a parameter
      ///        and copies it to the buffer before calling DateTime.toString() and returning the result.
      /// @note  The `DateTime::toString(char *buffer)` method reads the buffer parameter to extract the
      ///        formatting string before performing the conversion, placing the result back in the buffer and
      ///        returning a pointer to the same buffer. This forces the developer to litter their code with 
      ///        lines to initialize the buffer with the formatting string. This method changes that, you just 
      ///        pass the format string as a parameter and it is copied to the buffer before calling DateTime.toString().
      /// @param time The DateTime object to convert to a string.
      /// @param buffer The buffer to store the resulting string. Size the buffer to hold the resulting string, the
      ///               DateTime.toString() methods is vulnerable to buffer overrun.
      /// @param size The size of the buffer, not including the null '\0' terminator.
      /// @param format The format string to use for the conversion, default is "YYYY/MM/DD hh:mm:ss" (e.g. 2005/08/05 20:04:20).
      /// @return A pointer to the given `buffer` containing the resulting string, or nullptr if the buffer is null or size is 0.
      /// @author Chris-80 (2025/07)
      static char* DateTimeToString(DateTime time, char* buffer, size_t size, const char* format = "YYYY/MM/DD hh:mm:ss");

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

      #if STL_USED
      /// @brief Play a specific melody by reference to a `vector` of `Note` objects.
      /// @param melody A reference to the vector of `Note` objects to play.
      /// @see PlayMelody(size_t id)
      /// @see PlayAlarm(const AlarmTime& alarm)
      /// @author Chris-70 (2025/09)
      void PlayMelody(const std::vector<Note>& melody) const;
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
   
   //#################################################################################//  
   // Protected METHODS
   //#################################################################################//   
   protected:

      /// @brief Enum class to define the current hour color mode in use. Type: uint8_t
      enum class HourColor : uint8_t 
            { 
            Hour24 = 0,    ///< 24-hour mode, use OnHour colors for hours.
            Am,            ///< AM hour colors, when the AM indicator is OFF (i.e. Black)
            Pm             ///< PM hour colors, when the PM indicator is OFF (i.e. Black)
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

      /// @brief This helper method is called to service the user callback function with the associated time.
      /// @details This method is called when the RTC 1 Hz signal is triggered (time) or the alarm has triggered.
      /// @param triggerFlag The flag that indicates if the callback was fired.
      /// @param time The associated DateTime object to pass to the callback function (e.g. alarm time / current time).
      /// @param callback The user callback function to call with the associated DateTime.
      /// @author Chris-70 (2025/07)
      void CallbackFtn(volatile bool& triggerFlag, const DateTime& time, void (*callback)(const DateTime&));

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

      /// @brief The method called to display the given LED buffer on the shield.
      /// @details This method just copies the given `ledBuffer` contents directly to the 
      ///          FastLED buffer and displays it.
      /// @param ledBuffer The array buffer containing the LED colors to display.
      /// @author Chris-70 (2025/07)
      virtual void DisplayLedBuffer(const fl::array<CRGB, NUM_LEDS>& ledBuffer);

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

      #if HARDWARE_DEBUG
      /// @brief This method is called to check the hardware debug buttons/switches and set the serial output level.
      /// @author Chris-80 (2025/07)
      void CheckHardwareDebugPin();
      #endif

   //#################################################################################//  
   // Private METHODS
   //#################################################################################//   
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

      /// @brief Helper method to return the pointer to the `patternType` in the `ledPatterns_P` array.
      /// @param patternType The LED pattern type to display.
      /// @author Chris-70 (2025/08)
      const CRGB* patternLookup(LedPattern patternType);

      #if STL_USED
      /// @brief This method is called to initialize the default melody from the PROGMEM arrays.
      /// @details This method initializes the default melody from the PROGMEM array: `AlarmNotes`
      ///          This method is called from the constructor to ensure the `melodyRegistry` has
      ///          the default melody at index 0.
      void initializeDefaultMelody();
      #endif

      const fl::array<CRGB, NUM_HOUR_LEDS>& getCurHourColors();

      #if SERIAL_TIME_CODE
      /// @brief The method called to display the current time, decimal and binary, over the serial monitor.
      /// @details While this method can still be removed at compile time, it can also be controlled, at run-time, 
      ///          in software and hardware. This method is usually called every second so being able to control the         
      ///          output in software and hardware, by using a switch or jumper, can start/stop the serial time display.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void serialTime();
      #endif

      /// @brief Helper method to register a callback function for the time or alarm.
      /// @details This method is called by the public methods to register a callback function
      ///          for the time or alarm events. This method only works with a single pointer to
      ///          the callback function. The `cbFlag` is set to true when the callback is registered.
      /// @param callbackFtn The function to register as a callback.
      /// @param callback The callback function pointer to update.
      /// @param cbFlag The flag to set when the callback is registered.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see unregisterCallback()
      /// @author Chris-70 (2025/08)
      bool registerCallback(void (*callbackFtn)(const DateTime&), void (*&callback)(const DateTime&), bool& cbFlag);

      /// @brief Helper method to unregister a callback function for the time or alarm.
      /// @details This method is called by the public methods to unregister a callback function
      ///          for the time or alarm events. This method only works with a single pointer to
      ///          the callback function. The `cbFlag` is set to false when the callback is unregistered.
      /// @param callbackFtn The function to unregister as a callback.
      /// @param callback The callback function pointer to update.
      /// @param cbFlag The flag to set when the callback is unregistered.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see registerCallback()
      /// @author Chris-70 (2025/08)
      bool unregisterCallback(void (*callbackFtn)(const DateTime&), void (*&callback)(const DateTime&), bool& cbFlag);

   //#################################################################################//  
   // Public PROPERTIES   
   //#################################################################################//   
   public:

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

      //  ingroup properties
      /// @brief Singleton pattern to ensure only one instance of BinaryClock.
      /// @details Call this read only property to get a reference to the instance.
      /// @author Chris-80 (2025/07)
      static BinaryClock& get_Instance()
         {
         static BinaryClock instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }
         
      //  ingroup properties
      /// @brief Property pattern for the LED 'Brightness' property.
      ///        This property controls the brightness of the LEDs, 0-255, 20-30 is normal
      /// @param value The brightness level to set (0-255).
      /// @see get_Brightness()
      /// @author Chris-80 (2025/07)
      void set_Brightness(byte value);
      //  ingroup properties
      /// @copydoc set_Brightness()
      /// @return The current brightness level (0-255).
      /// @see set_Brightness()
      byte get_Brightness();

      //  ingroup properties
      /// @brief Property pattern for the 'IsSerialSetup' flag property. 
      ///        This property controls whether the serial setup menu is displayed or not.
      /// @param value The flag to set (true to display the serial setup menu, false to disable it).
      /// @see get_IsSerialSetup()
      /// @author Chris-80 (2025/07)
      void set_IsSerialSetup(bool value);

      //  ingroup properties
      /// @brief Property pattern for the 'IsSerialTime' flag property.
      ///        This property controls whether the serial time is displayed or not.
      /// @remarks This will override hardware control requiring the switch or button be
      ///          pressed again to regain hardware control (if H/W is available).
      /// @param value The flag to set (true to display the serial time, false to disable it).
      /// @return The current flag value (true to display the serial time, false to disable it).
      /// @see get_IsSerialTime()
      /// @author Chris-80 (2025/07)
      void set_IsSerialTime(bool value);

      //  ingroup properties
      /// @brief Property pattern for the 'OnColors' property.
      ///        This property controls the colors of the LEDs when they are on.
      /// @param value A reference the array of colors to set for the LEDs when they are on.
      /// @see get_OnColors()
      /// @author Chris-70 (2025/08)
      void set_OnColors(const fl::array<CRGB, NUM_LEDS>& value);
      /// @copydoc set_OnColors()
      /// @return A const reference to the array of colors for the LEDs when they are on.
      /// @see set_OnColors()
      const fl::array<CRGB, NUM_LEDS>& get_OnColors() const;

      //  ingroup properties
      /// @brief Property pattern for the 'OffColors' property.
      ///        This property controls the colors of the LEDs when they are off.
      /// @param value A reference the array of colors to set for the LEDs when they are off.
      /// @note Normally this color is CRGB::Black (i.e. LED is OFF). Any other color will   
      ///       keep the LED on at all times, always consuming power.
      /// @see get_OffColors()
      /// @author Chris-70 (2025/08)
      void set_OffColors(const fl::array<CRGB, NUM_LEDS>& value);
      /// @copydoc set_OffColors()
      /// @return A const reference to the array of colors for the LEDs when they are off.
      /// @see set_OffColors()
      const fl::array<CRGB, NUM_LEDS>& get_OffColors() const;

      /// @property OnHourPM
      /// @brief Property pattern for the 'OnHourPM' property.
      ///        This property controls the colors of the LEDs when they are ON for the hour display.
      /// @details These values are always used for the HOUR LEDs except when `AmColor` is CRGB::Black 
      ///          AND `Is12HourFormat` is true.
      /// @param value A reference the array of colors to set for the LEDs when they are on for the hour display.
      /// @return A const reference to the array of colors for the LEDs when they are on for the hour display.
      /// @see get_OnHourPM()
      /// @see set_OnHourAM()
      /// @see get_OnHourAM()
      /// @author Chris-70 (2025/08)
      //  ingroup properties
      /// @{
      void set_OnHourPM(const fl::array<CRGB, NUM_HOUR_LEDS>& value);
      const fl::array<CRGB, NUM_HOUR_LEDS>& get_OnHourPM() const;
      /// @}

      //  ingroup properties
      /// @brief Property pattern for the `OnHourAM` property.
      ///        This property controls the colors of the LEDs when they are on for the hour display in AM mode.
      /// @details These color are ONLY used when the `AmColor` is CRGB::Black AND `Is12HourFormat` is true.
      ///          This is to be able to distinguish between 12 midnight in 12 hour mode and 12 noon in 24 hour mode.
      /// @param value A reference the array of colors to set for the LEDs when they are on for the hour display in AM mode.
      /// @return A const reference to the array of colors for the LEDs when they are on for the hour display in AM mode.
      /// @see set_OnHourPM()
      /// @see get_OnHourPM()
      /// @see get_OnHourAM()
      /// @author Chris-70 (2025/08)
      void set_OnHourAM(const fl::array<CRGB, NUM_HOUR_LEDS>& value);
      /// @copydoc set_OnHourAM()
      /// @see set_OnHourAM()
      const fl::array<CRGB, NUM_HOUR_LEDS>& get_OnHourAM() const;

      //  ingroup properties
      /// @brief Property pattern for the 'AmColor' property, used when @see Is12HourFormat is true.
      ///        This property controls the color of the AM indicator LED in 12 hour mode.
      /// @param value The color to set for the AM indicator LED when in 12 hour mode.
      /// @remark This default value is not Black to indicate 12 hour mode. Default is CRGB::DeepSkyBlue
      /// @sideeffect The @see OnColor values for the Hour Row  change when moving to/from Black.
      ///        If AmColor is set to Black, the OnHourAM colors are copied to the Hour Row 
      ///        for the AM hours and @see OnHourPM colors are copied to OnColors hours for the PM.
      ///        This is to distinguish between 12 midnight in 12 hour mode and 12 noon in 24 hour mode.
      /// @return The current color of the AM indicator LED when in 12 hour mode.
      /// @see get_AmColor()
      /// @author Chris-70 (2025/08)
      void set_AmColor(CRGB value);
      /// @copydoc set_AmColor()
      /// @see set_AmColor()
      CRGB get_AmColor() const;

      //  ingroup properties
      /// @brief Property pattern for the 'PmColor' property, used when @see Is12HourFormat is true.
      ///        This property controls the color of the PM indicator LED in 12 hour mode.
      /// @param value The color to set for the PM indicator LED when in 12 hour mode.
      /// @return The current color of the PM indicator LED when in 12 hour mode.
      /// @remark This value is normally not Black. Default is CRGB::Indigo
      /// @see get_PmColor()
      /// @author Chris-70 (2025/08)
      void set_PmColor(CRGB value);
      /// @copydoc set_PmColor()
      /// @see set_PmColor()
      CRGB get_PmColor() const;

      //  ingroup properties
      /// @brief Property: 'DebounceDelay' time (ms) for the button press to stabilize. 
      ///        Initially set to  DEFAULT_DEBOUNCE_DELAY.
      /// @param value The debounce delay time in milliseconds.
      /// @return The current debounce delay time in milliseconds.
      /// @see get_DebounceDelay()
      /// @author Chris-70 (2025/07)
      void set_DebounceDelay(unsigned long value);
      /// @copydoc set_DebounceDelay()
      /// @see set_DebounceDelay()
      unsigned long get_DebounceDelay() const;
      
      #if STL_USED
      //  ingroup properties
      /// @brief Property pattern for the 'MelodyNumber' property using id number to melodyRegistry.
      /// @details This property allows setting and getting the current melody by its registry number.
      ///          Number 0 is always the default melody created from PROGMEM arrays.
      /// @param value The id number of a registered melody to set as the current melody.
      /// @note The melody id number must be valid (number returned from RegisterMelody()) or 
      ///       the setter will ignore it.
      /// @see get_Melody()
      /// @see RegisterMelody()
      /// @see get_CurrentMelody()
      /// @author Chris-70 (2025/09)
      void set_Melody(size_t value);
      /// @copydoc set_Melody()
      /// @return The current melody registry id number.
      /// @see set_Melody()
      size_t get_Melody() const;
   
      //  ingroup properties
      /// @brief  Read only property: Get the current melody vector by reference.
      /// @return A const reference to the current melody vector.
      /// @see get_Melody()
      /// @author Chris-70 (2025/09)
      const std::vector<Note>& get_CurrentMelody() const;
   
      //  ingroup properties
      /// @brief Read only property: Get the number of registered melodies.
      /// @return The number of melodies in the registry.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      size_t get_MelodyCount() const { return melodyRegistry.size(); }
      #endif

      #if HW_DEBUG_TIME
      //  ingroup properties
      /// @brief Property pattern for the 'DebugOffDelay' property. This controls how fast 
      ///        the serial time monitor is turned off after the debug pin goes OFF.
      /// @see get_DebugOffDelay()
      /// @author Chris-80 (2025/07)
      void set_DebugOffDelay(unsigned long value);
      /// @copydoc set_DebugOffDelay()
      /// @see set_DebugOffDelay()
      unsigned long get_DebugOffDelay() const;
      #endif
      /// @}

   //#################################################################################//  
   // Protected PROPERTIES   
   //#################################################################################//   
   protected:
      void set_TimeFormat(const char* value)
         { TimeFormat = (value == nullptr? get_TimeFormat() : value); }

      void set_AlarmFormat(const char* value)
         { AlarmFormat = (value == nullptr? get_AlarmFormat() : value); }

   //#################################################################################//  
   // Private PROPERTIES   
   //#################################################################################//   
   private:
   TEST_PROTECTED

   // ################################################################################
   // Public FIELDS
   // ################################################################################
   public:         

      /// @brief The pin number to use for the heartbeat (if enabled) or to signal errors.
      /// @note  The LED must be wired CC, the pin will go HIGH to turn it ON.
      static uint8_t HeartbeatLED;

   // ################################################################################
   // Protected FIELDS
   // ################################################################################
   protected:
      RTCLibPlusDS3231 RTC;                        ///< Create RTC object using forked Adafruit RTCLib library

      /// @brief Default: Colors for the LEDs when ON, Seconds, Minutes and Hours
      /// @details The default colors are Hours: Blue; Minutes: Green; and Seconds: Red
      /// @note The hours are defined by `OnHour` color array. These are the colors used for 24 hour mode and for PM.
      ///      AM is defined by the `OnHourAM` color array when `AmColor` is Black.  This is to remove ambiguity for
      ///      hour 12. Is it noon in 24 hour mode or midnight in 12 hour mode? Without an AM indicator, who knows?
      static fl::array<CRGB, NUM_LEDS> OnColor;

      /// @brief Default: Colors for the LEDs Seconds, Minutes and Hours, when OFF (Usually Black i.e. No Power.)
      /// @note  Using any color other than Black means the LEDs will be consuming power at all times.
      static fl::array<CRGB, NUM_LEDS> OffColor;

      /// @brief Default: Colors for the AM hour LEDs when `AmColor` is `Black` in 12 hour mode.
      ///        This is used when there is no AM indicator. 
      /// @remarks When the AM indicator color is Black, there is no way to differentiate between 12 noon in 
      ///          24 hour mode and 12 midnight in 12 hour mode. To remove this ambiguity, the AM hours are shown
      ///          in a different color, e.g. DeepSkyBlue.
      /// @see OnHour
      static fl::array<CRGB, NUM_HOUR_LEDS> OnHourAM; 

      /// @brief Default: LED colors for the PM hours when `PmColor` is `Black` in 12 hour mode
      ///        This is used when there is no PM indicator. 
      /// @remarks When the PM indicator color is Black, there is no way to differentiate between 12 noon in 
      ///          24 hour mode and 12 midnight in 12 hour mode. To remove this ambiguity, the PM hours are shown
      ///          in a different color, e.g. Indigo.
      static fl::array<CRGB, NUM_HOUR_LEDS> OnHourPM;

      static CRGB PmColor;                         ///< Color for the PM indicator LED. Default is Indigo.
      static CRGB AmColor;                         ///< Color for the AM indicator LED. Default is SkyBlue.

      // The UNO Compiler doesn't support this C++ style object initialization, move it to the constructor.
      AlarmTime Alarm1;                            ///< DS3232 alarm 2, includes seconds in alarm.
      AlarmTime Alarm2;                            ///< Default alarm, seconds set at 00.

      volatile bool RTCinterruptWasCalled;         ///< Flag: The RTC interrupt was triggered.
      volatile bool CallbackAlarmTriggered;        ///< Flag: The 'Alarm' callback needs to be called.
      volatile bool CallbackTimeTriggered;         ///< Flag: The 'Time'  callback needs to be called.

      const char* TimeFormat = timeFormat24;       ///< Pointer to the current format string for the time.
      const char* AlarmFormat = alarmFormat24;     ///< Pointer to the current format string for the alarm.

   // ################################################################################
   // private FIELDS
   // ################################################################################
   private:
   TEST_PROTECTED

      /// @brief 2D table array to map the `AlarmTime::Repeat` enumerations with
      ///        the corresponding enumeration for Alarm1 and Alarm2.
      /// @details The alarms each have different enumeration values for the
      ///          alarm repetations so this array provides a way to map a common
      ///          repeat enumeration with the different alarms on the hardware.
      /// @note The `Repeat::endTag` must be the last value as it is used to define the array size.
      static const uint8_t repeatModeTable[static_cast<uint8_t>(AlarmTime::Repeat::endTag)][2];
      #define REPEAT_MODE_ROW_COUNT (sizeof(repeatModeTable) / sizeof(repeatModeTable[0]))
      static_assert(REPEAT_MODE_ROW_COUNT == (uint8_t)(AlarmTime::Repeat::endTag), "Repeat mode table size mismatch");

      /// @brief This is just a copy of the hour portion of the `OnColor` array.
      /// @details This is used for the hour colors in 24 hour mode and when the AM or PM indicators are ON.  
      ///          When either the AM or PM indicator is Black, the corresponding `OnHourAM` or `OnHourPM`
      ///          color will be used instead.
      /// @remarks This was added to make the code simpler so that the hour colors are all the same type, 
      ///          `fl::array<CRGB, NUM_HOUR_LEDS>`, instead of using pointers for the current hour colors.
      static fl::array<CRGB, NUM_HOUR_LEDS> onHour24;

      const char* timeFormat24 = "hh:mm:ss";       ///< 24-hour time format string: 00:00:00 to 23:59:59
      const char* timeFormat12 = "HH:mm:ss AP";    ///< 12-hour time format string: 12:00:00 AM to 11:59:59 PM
      const char* alarmFormat24 = "hh:mm";         ///< 24-hour alarm format string: 00:00 to 23:59
      const char* alarmFormat12 = "HH:mm AP";      ///< 12-hour alarm format string: 12:00 AM to 11:59 PM

      CRGB leds[NUM_LEDS] = {0};                   ///< Array of LED colors to display the current time
      bool binaryArray[NUM_LEDS];                  ///< Serial Debug: Array for binary representation of the time

      fl::array<CRGB, NUM_LEDS>& onColors;         ///< Reference to the current ON  colors.
      fl::array<CRGB, NUM_LEDS>& offColors;        ///< Reference to the current OFF colors.
      fl::array<CRGB, NUM_HOUR_LEDS>& onHourPM;    ///< Reference to the color array for the PM hours.
      fl::array<CRGB, NUM_HOUR_LEDS>& onHourAM;    ///< Reference to the color array for the AM hours.
      // fl::array<CRGB, NUM_HOUR_LEDS>& onHour24;    ///< Reference to the color array for the standard or 24-hour format.
      fl::array<CRGB, NUM_HOUR_LEDS>& onHour =  onHour24;///< Reference to the color array for the current hour colors in use.

      BCButton buttonS1;  ///< S1 button (Time/Decrement)
      BCButton buttonS2;  ///< S2 button (Save/Stop)
      BCButton buttonS3;  ///< S3 button (Alarm/Increment)
      
      #if HW_DEBUG_SETUP
      BCButton buttonDebugSetup; ///< Debug setup button
      #endif
      #if HW_DEBUG_TIME
      BCButton buttonDebugTime;  ///< Debug time button
      #endif

      BCMenu settings;       ///< Settings handler instance

      DateTime time;                         ///< Current time from the RTC, updated every second.
      bool amPmMode = false;                 ///< Flag: Indicates if the clock is in 12-hour AM/PM, or 24 Hr mode.
      bool callbackAlarmEnabled = false;     ///< Flag: The 'Alarm' callback is enabled (i.e. is not nullptr) or not.
      bool callbackTimeEnabled  = false;     ///< Flag: The 'Time'  callback is enabled (i.e. is not nullptr) or not.
      bool rtcValid             = false;     ///< Flag: The RTC was found and initialized.
      void (*alarmCallback)(const DateTime&) = nullptr; ///< Callback function for alarm triggers.
      void (*timeCallback)(const DateTime&)  = nullptr; ///< Callback function for time trigger (1 Hz frequency).

      unsigned long debounceDelay = DEFAULT_DEBOUNCE_DELAY; ///< The debounce time for a button press.
      bool pixelsPresent = false;            ///< Flag: Indicates if the shield is attached (or just a dev. board).

      // static const uint8_t repeatModeTable[(uint8_t)(AlarmTime::Repeat::endTag)][2];

      int alarmRepeatMax = DEFAULT_ALARM_REPEAT;   ///< Maximum alarm sound repeat count
      int alarmRepeatCount = 0;                    ///< Current alarm sound repeat count
      byte brightness = DEFAULT_BRIGHTNESS;        ///< Brightness of the LEDs, 0-255 (20 - 60 is a good range).

      char buffer[64] = { 0 };                     ///< Buffer for the DateTime string conversions

      bool isSerialSetup = (SERIAL_SETUP_CODE) && (DEFAULT_SERIAL_SETUP); ///< Serial setup flag
      bool isSerialTime  = (SERIAL_TIME_CODE)  && (DEFAULT_SERIAL_TIME);  ///< Serial time  flag 

      bool isAmBlack = (AmColor == CRGB::Black);  ///< Flag: Controls if we switch the hour colors for AM/PM.
      bool isPmBlack = (PmColor == CRGB::Black);  ///< Flag: Controls if we switch the hour colors for AM/PM.
      bool switchColors = false; ///< Flag to perform the switch of OnHour and OnHourAM hour colors.
      HourColor curHourColor = HourColor::Hour24; ///< Current ON hosur colors in use.

      /// @var CRGB ledPatterns_P[][]
      /// @brief A 2D array of LED colors and patterns. stored in flash memory, for the shield.
      ///          These are the default ON/OFF colors for displaying the time as well as
      ///          all the patterns used in the settings menu for time and alarm.
      /// @details The enum `LedPattern` is the index to the color/pattern for that display.
      /// @par     **`LedPattern::onColors`**:  
      ///          **`OnColors`** The default colors are Hours: Blue; Minutes: Green; and Seconds: Red 
      ///          for the LEDs when ON. These values are always used in 24 hour mode.  
      ///          In 12 hour mode, the left most hour bit, MSB, is used for the AM / PM indicators: 
      ///          **`AmColor`** and **`PmColor`**. Alternativly different color arrays can be used for the 
      ///          hours in AM and PM: **`OnHourAM`** for AM and **`OnHour`** for PM. These are used when 
      ///          either of the respective `AmColor` or `PmColor` indicatore colors are Black.   
      /// @par     `LedPattern::offColors`:    
      ///          `OffColors` for the LEDs when OFF (Usually Black; no power).
      ///          Using any color other than Black means the LEDs will be consuming power at all times.
      /// @par     `LedPattern::onText`:   
      ///          `OnText` The screen shaped in an **`O`** for 'On' when setting the alarm to 
      ///          ON in the alarm menu.
      /// @par     `LedPattern::offTxt`:   
      ///          `OffTxt` The screen shaped in a RED sideways **`F`** for 'oFF' when setting the alarm to OFF in the alarm menu.
      /// @par     `LedPattern::xAbort`:   
      ///          `XAbort` The screen shaped in a big Pink (Fuchsia) **`X`** [❌] for abort/cancel.
      ///          This is used to cancel the Time and Alarm settings and exit without making any changes.
      ///          This is also displayed, after the Rainbow (saving/exit) screen to signal nothing saved.
      /// @par     `LedPattern::okText`:   
      ///          `OkText` The screen shaped in a big Lime **`✓`** [✅] for okay/good.
      ///          This is used to signal that the settings have been saved successfully.
      /// @par     `LedPattern::rainbow`:   
      ///          `Rainbow` The screen shaped in a big Rainbow of colors across all LEDs.
      ///          This is displayed after the Time or Alarm settings has ended and the
      ///          program is saving/restoring the settings. This is followed by either
      ///          the **`✓`** [✅] for settings saved or the **`X`** [❌] for no changes.
      /// @par     `LedPattern::wText`:   
      ///          `WText` The screen shaped in a big Blue **`W`** [📶] for WPS / WiFi
      ///          This is used to signal that the WiFi needs to setup (e.g. WPS) and is only available
      ///          when the device supports WiFi (i.e. ESP32_WIFI or WIFIS3 are defined).
      /// @see `LedPattern`
      /// @see `AmColor`
      /// @see `PmColor`
      static const CRGB ledPatterns_P[static_cast<uint8_t>(LedPattern::endTAG)][NUM_LEDS] PROGMEM;

      /// @var CRGB hourColors_P[][]
      /// @brief A 2D array for colors for just the hours. The `OnHourAM` is the alternative colors used 
      ///        for the AM hour LEDs when `AmColor` is Black. This is to show the clock is in 12 hour
      ///        mode when there is no AM indicator. The `OnHour` is the hour colors used when `PmColor` is Black.
      /// @details The `OnColor` hour section is replaced with either the `OnHourAM` or `OnHour` colors
      ///          when the time switches from AM to PM and the flag `isAmBlack` is `true`.
      /// @remarks When the AM or PM indicator color is Black, there is no way to differentiate between 12 noon in 
      ///          24 hour mode and 12 midnight in 12 hour mode. To remove this ambiguity, the AM/PM hours are shown
      ///          in a different color, e.g. DeepSkyBlue or Indigo.
      /// @par     OnHourAM:   
      ///          Index 0 - `OnHourAM`
      /// @par     OnHour:   
      ///          Index 1 - `OnHourPM`
      static const CRGB hourColors_P[][NUM_HOUR_LEDS] PROGMEM;

      static const uint8_t ledPatternCount;  ///< Number of patterns in the 2D array (i.e. value of LedPattern::endTAG).

      static const CRGB* onColor_P;     ///< Pointer to the `OnColor`  colors (index 0)  in `ledPatterns_P`
      static const CRGB* offColor_P;    ///< Pointer to the `OffColor` colors (index 1)  in `ledPatterns_P`
      static const CRGB* onHourAm_P;    ///< Pointer to the `OnHourAM` colors (index 0)  in `hourColors_P`
      static const CRGB* onHourPm_P;    ///< Pointer to the `OnHourPM` colors (index 1)  in `hourColors_P`
      static const CRGB* onHour24_P;    ///< Pointer to the `OnHour24` colors (index 12) in `onColor_P`

      /// @brief Time to wait after serial time button goes off before stopping the serial output.
      ///        Set to a long delay if using a momentary button, keep short for a switch. This
      ///        allows a button to be pressed, released and you still get output for 'debugDelay' ms.
      unsigned long debugDelay = DEFAULT_DEBUG_OFF_DELAY; 

      #if STL_USED
      std::vector<Note> defaultMelody;                    ///< Default melody created from PROGMEM arrays
      std::vector<std::reference_wrapper<const std::vector<Note>>> melodyRegistry; ///< Registry of melody references
      size_t currentMelody;                               ///< Index to the current melody in melodyRegistry
      #else
      bool isDefaultMelody = true;     ///< Flag: using the default (Flash ROM) alarm melody.
      const Note* alarmNotes;          ///< Pointer to the combined alarm notes array
      size_t alarmNotesSize;           ///< Size of the alarm notes array
      #endif
   
      static const Note AlarmNotes[] PROGMEM;     ///< The default alarm melody, an array of `Notes`
      static const size_t AlarmNotesSize;         ///< Size of the AlarmNotes array

      const char* IBinaryClock_IdName = "BinaryClock_v0.8";
      }; // END Class BinaryClock
   }  // END namespace BinaryClockShield

#endif // END __BINARY_CLOCK_H__
